//
// Created by André Schnabel on 24.10.15.
//

#include <list>
#include <localsolver.h>
#include "LSSolver.h"
#include <fstream>
#include <iostream>

using namespace localsolver;

class SchedulingNativeFunction : public LSNativeFunction {
protected:
    ProjectWithOvertime &p;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
    ~SchedulingNativeFunction() {}
};

class RevenueFunction : public SchedulingNativeFunction {
public:
	explicit RevenueFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
    virtual lsdouble call(const LSNativeContext &context) override;
};

class CumulatedDemandFunction : public SchedulingNativeFunction {
public:
	explicit CumulatedDemandFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) { }
    virtual lsdouble call(const LSNativeContext &context) override;
};

lsdouble RevenueFunction::call(const LSNativeContext &context) {
    return p.revenue[context.getIntValue(0)];
}

lsdouble CumulatedDemandFunction::call(const LSNativeContext &context) {
    int demand = 0;

    lsint r = context.getIntValue(0);
    lsint t = context.getIntValue(1);

    p.eachJob([&](int j) {
        lsint Sj = context.getIntValue(j + 2);
        if (Sj < t && t <= Sj + p.durations[j])
            demand += p.demands(j, static_cast<int>(r));
    });

    return demand;
}

vector<int> LSSolver::solve3(ProjectWithOvertime &p) {
	vector<int> sts(p.numJobs);

	LocalSolver ls;
	auto model = ls.getModel();

	// Decision variables (primary)
	vector<LSExpression> S(p.numJobs);
    p.eachJob([&](int j) { S[j] = model.intVar(0, p.numPeriods - 1); });

	// Intermediate expressions
	Matrix<LSExpression> cumDemandExpr(p.numRes, p.numPeriods);
    p.eachResPeriod([&](int r, int t) {
        cumDemandExpr(r,t) = model.sum();
        p.eachJob([&](int j) { cumDemandExpr(r,t) += p.demands(j,r) * (t > S[j] && t <= S[j] + p.durations[j]); });
    });


	// Revenue function parameter
	LSExpression revenueArray = model.array();
	p.eachPeriod([&](int t){revenueArray.addOperand(p.revenue[t]); });

	// Objective function
	LSExpression objfunc = model.sum();
	objfunc += model.at(revenueArray, S[p.numJobs - 1]);
    p.eachResPeriod([&](int r, int t) {objfunc += -p.kappa[r] * model.max(0, cumDemandExpr(r,t) - p.capacities[r]); });

	// Precedence restriction
    p.eachJob([&](int j) {
        LSExpression lastPred = model.max(0);
        for (int i = 0; i < p.numJobs; i++) {
            if (p.adjMx(i, j)) lastPred.addOperand(S[i] + p.durations[i]);
        }
        model.constraint(lastPred <= S[j]);
    });

	// Capacity restriction
    p.eachResPeriod([&](int r, int t) { model.constraint(cumDemandExpr(r,t) <= p.capacities[r] + p.zmax[r]); });

	model.addObjective(objfunc, OD_Maximize);
	model.close();

	ls.createPhase().setTimeLimit(5);
	auto param = ls.getParam();
	param.setNbThreads(8);
	param.setVerbosity(2);
	ls.solve();

	auto sol = ls.getSolution();
	p.eachJob([&](int j) { sts[j] = static_cast<int>(sol.getIntValue(S[j])); });

	//auto status = sol.getStatus();
	//auto solvetime = ls.getStatistics().getRunningTime();

	return sts;
}

vector<int> LSSolver::solve2(ProjectWithOvertime &p) {
	vector<int> sts(p.numJobs);

	LocalSolver ls;
	auto model = ls.getModel();

	// Decision variables
	vector<LSExpression> S(p.numJobs);
	p.eachJob([&](int j) { S[j] = model.intVar(0, p.numPeriods - 1); });

	Matrix<LSExpression> zrt(p.numRes, p.numPeriods);
	p.eachResPeriod([&](int r, int t) { zrt(r,t) = model.intVar(0, p.zmax[r]); });

	// Revenue function parameter
	LSExpression revenueArray = model.array();
    p.eachPeriod([&](int t) { revenueArray.addOperand(p.revenue[t]); });

	// Objective function
	LSExpression objfunc = model.sum();
	objfunc += model.at(revenueArray, S[p.numJobs - 1]);
	p.eachResPeriod([&](int r, int t) { objfunc += -p.kappa[r] * zrt(r,t); });
	
	// Precedence restriction
	p.eachJob([&](int j) {
		LSExpression lastPred = model.max(0);
        for(int i=0; i<p.numJobs; i++)
		    if (p.adjMx(i,j)) lastPred.addOperand(S[i] + p.durations[i]);
		model.constraint(lastPred <= S[j]);
    });

	// Capacity restriction
	p.eachResPeriod([&](int r, int t) {
        LSExpression cumDemandExpr = model.sum();
        p.eachJob([&](int j) { cumDemandExpr += p.demands(j, r) * (t > S[j] && t <= S[j] + p.durations[j]); });
        model.constraint(cumDemandExpr <= p.capacities[r] + zrt(r, t));
    });

    model.addObjective(objfunc, OD_Maximize);
	model.close();

	ls.createPhase().setTimeLimit(5);
	auto param = ls.getParam();
	param.setNbThreads(8);
	param.setVerbosity(2);
	ls.solve();

	auto sol = ls.getSolution();
    p.eachJob([&](int j){ sts[j] = static_cast<int>(sol.getIntValue(S[j])); });

	//auto status = sol.getStatus();
	//auto solvetime = ls.getStatistics().getRunningTime();

	return sts;
}

vector<int> LSSolver::solve(ProjectWithOvertime &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    RevenueFunction revFunc(p);
    LSExpression revFuncExpr = model.createNativeFunction(&revFunc);

    CumulatedDemandFunction cumDemFunc(p);
    LSExpression cumDemFuncExpr = model.createNativeFunction(&cumDemFunc);

    // Decision variables
    vector<LSExpression> Sj(p.numJobs);
    p.eachJob([&](int j) {Sj[j] = model.intVar(0, p.numPeriods-1); });
    Matrix<LSExpression> zrt(p.numRes, p.numPeriods);
    p.eachResPeriod([&](int r, int t) { zrt(r,t) = model.intVar(0, p.zmax[r]); });

    // Objective function
    LSExpression objfunc = model.sum();
    LSExpression revenueExpr = model.createExpression(O_Call, revFuncExpr);
    revenueExpr.addOperand(Sj[p.numJobs-1]);
    objfunc += revenueExpr;
    p.eachResPeriod([&](int r, int t) { objfunc += -p.kappa[r] * zrt(r,t); });

    // Precedence restriction
    p.eachJob([&](int j) {
        LSExpression lastPred = model.max(0);
        for(int i=0; i<p.numJobs; i++)
            if(p.adjMx(i,j)) lastPred.addOperand(Sj[i] + p.durations[i]);
        model.constraint(lastPred <= Sj[j]);
    });

    // Capacity restriction
	p.eachResPeriod([&](int r, int t) {
        LSExpression cumDemandExpr = model.createExpression(O_Call, cumDemFuncExpr);
        cumDemandExpr.addOperand(r);
        cumDemandExpr.addOperand(t);
        p.eachJob([&](int j) { cumDemandExpr.addOperand(Sj[j]); });
        model.constraint(cumDemandExpr <= p.capacities[r] + zrt(r,t));
    });

    model.addObjective(objfunc, OD_Maximize);
    model.close();

    ls.createPhase().setTimeLimit(30);
	auto param = ls.getParam();
	param.setNbThreads(8);
	param.setVerbosity(2);
    ls.solve();

    auto sol = ls.getSolution();
    p.eachJob([&](int j) { sts[j] = static_cast<int>(sol.getIntValue(Sj[j])); });

    //auto status = sol.getStatus();
    //auto solvetime = ls.getStatistics().getRunningTime();

    return sts;
}

vector<int> LSSolver::solveMIPStyle(ProjectWithOvertime &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    // Decision variables
    Matrix<LSExpression> x(p.numJobs, p.numPeriods), z(p.numRes, p.numPeriods);
    p.eachJob([&](int j) { p.eachPeriod([&](int t){ x(j,t) = model.boolVar(); }); });
    p.eachResPeriod([&](int r, int t) { z(r,t) = model.intVar(0, p.zmax[r]); });

    // Objective function
    auto objfunc = model.sum();
    p.timeWindow(p.numJobs-1, [&](int t) { objfunc += p.revenue[t] * x(p.numJobs-1,t); });
    p.eachResPeriod([&](int r, int t) { objfunc += -p.kappa[r] * z(r,t); });

    // Constraints

    // Each job once
    p.eachJob([&](int j) {
        auto xsum = model.sum();
        p.timeWindow(j, [&](int t){ xsum += x(j,t); });
        model.constraint(xsum == 1.0);
    });

    // Precedence restrictions
    p.eachJobPair([&](int i, int j) {
        if(p.adjMx(i,j)) {
            auto predFt = model.sum();
            p.timeWindow(i, [&](int t) { predFt += t * x(i,t); });
            auto jobSt = model.sum();
            p.timeWindow(j, [&](int t) { jobSt += t * x(j,t); });
            jobSt += -p.durations[j];
            model.constraint(predFt <= jobSt);
        }
    });
            

    // Capacity restrictions
    p.eachRes([&](int r){
        p.eachPeriod([&](int t){
            auto cumulatedDemand = model.sum();
            p.eachJob([&](int j) {
                for(int tau = t; tau < Utils::min(t + p.durations[j], p.numPeriods); tau++)
                    cumulatedDemand += x(j,tau) * p.demands(j,r); });
            auto totalCapacity = model.sum(p.capacities[r], z(r,t));
            model.constraint(cumulatedDemand <= totalCapacity); }); });

    model.addObjective(objfunc, OD_Maximize);
    model.close();

    ls.createPhase().setTimeLimit(2);
	auto param = ls.getParam();
	param.setNbThreads(8);
	param.setVerbosity(2);
    ls.solve();

    auto sol = ls.getSolution();
    p.eachJob([&](int j) {
        for(int t=0; t<p.numPeriods; t++) {
            if(sol.getValue(x(j,t)) == 1) {
                sts[j] = t - p.durations[j];
                break;
            }
        }
    });

    auto status = sol.getStatus();
    if(status != SS_Feasible) {
        throw runtime_error("No feasible solution found!");
    }

    auto solvetime = ls.getStatistics().getRunningTime();
    cout << "Solvetime = " << solvetime << endl;

    return sts;
}

void LSSolver::writeLSPModelParamFile(ProjectWithOvertime &p, string outFilename) {
	list<string> outLines = {
		"njobs/nperiods/nres",
		to_string(p.numJobs),
		to_string(p.numPeriods),
		to_string(p.numRes) };

	outLines.push_back("durations");
    p.eachJob([&](int j) { outLines.push_back(to_string(p.durations[j])); });
	outLines.push_back("demands (j,r)-matrix (row major order)");
    p.eachJob([&](int j) { p.eachRes([&](int r){ outLines.push_back(to_string(p.demands(j,r))); }); });
	outLines.push_back("adjacency matrix (row major order)");
	p.eachJobPair([&](int i, int j) { outLines.push_back(p.adjMx(i,j) ? "1" : "0"); });
	outLines.push_back("capacities");
    p.eachRes([&](int r){ outLines.push_back(to_string(p.capacities[r])); });

	outLines.push_back("earliest finishing times");
    p.eachJob([&](int j) { outLines.push_back(to_string(p.efts[j])); });
	outLines.push_back("latest finishing times");
    p.eachJob([&](int j) { outLines.push_back(to_string(p.lfts[j])); });

	outLines.push_back("revenue");
    for(int t=0; t<p.numPeriods+1; t++) {
        outLines.push_back(to_string(p.revenue[t]));
    }
	outLines.push_back("upper bound for overtime");
    p.eachRes([&](int r){ outLines.push_back(to_string(p.zmax[r])); });
	outLines.push_back("kappa");
    p.eachRes([&](int r){ outLines.push_back(to_string(p.kappa[r])); });

	ofstream f(outFilename);
	if (!f.is_open()) return;
	for(auto onum : outLines)
		f << onum << "\n";
	f.close();
}