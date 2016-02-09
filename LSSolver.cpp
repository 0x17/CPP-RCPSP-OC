//
// Created by André Schnabel on 24.10.15.
//

#include <list>
#include <fstream>
#include <iostream>

#include "LSSolver.h"
#include "Stopwatch.h"

//======================================================================================================================

ListModel::~ListModel() {
	if(decoder != nullptr)
		delete decoder;
}

vector<int> ListModel::solve(SolverParams params) {
	buildModel();
	applyParams(params);
	ls.solve();
	auto sol = ls.getSolution();
	return parseScheduleFromSolution(sol);
}

void ListModel::buildModel() {
	if (model.getNbObjectives() == 0) {
		LSExpression obj = model.createExpression(O_Call, model.createNativeFunction(decoder));

		LSExpression activityList = model.listVar(p.numJobs);
		model.constraint(model.count(activityList) == p.numJobs);

		for (int i = 0; i < p.numJobs; i++) {
			listElems[i] = model.at(activityList, i);
			obj.addOperand(listElems[i]);
		}

		addAdditionalData(obj);

		model.addObjective(obj, OD_Maximize);
		model.close();
	}
}

void ListModel::applyParams(SolverParams &params) {
	if (ls.getNbPhases() == 0) {
		ls.createPhase().setTimeLimit(static_cast<int>(params.timeLimit));
		auto param = ls.getParam();
		param.setNbThreads(params.threadCount);
		param.setSeed(params.seed);
		param.setVerbosity(params.verbosityLevel);
	}
}

//======================================================================================================================

lsdouble ListAlternativesModel::SerialSGSAlternativesDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	if (context.count() < p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
	}
	auto result = p.serialSGSWithOvertime(order, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

vector<int> ListAlternativesModel::parseScheduleFromSolution(LSSolution &sol) {
	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
	}
	return p.serialSGSWithOvertime(order, true).first;
}

lsdouble ListBetaModel::SerialSGSBetaFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs), beta(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		beta[i] = static_cast<int>(context.getIntValue(p.numJobs + i));
	}
	auto result = p.serialSGSTimeWindowBorders(order, beta, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListBetaModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		betaVar[i] = model.boolVar();
		obj.addOperand(betaVar[i]);
	}
}

vector<int> ListBetaModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs), betaVarVals(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		betaVarVals[i] = static_cast<int>(sol.getIntValue(betaVar[i]));
	}
	return p.serialSGSTimeWindowBorders(order, betaVarVals, true).first;
}

lsdouble ListTauModel::SerialSGSTauFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		tau[i] = static_cast<float>(context.getDoubleValue(p.numJobs + i));
	}
	auto result = p.serialSGSTimeWindowArbitrary(order, tau, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

lsdouble ListFixedOvertimeModel::SerialSGSZrDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs), zr(p.numRes);
	if (context.count() < p.numJobs + p.numRes) return numeric_limits<double>::lowest();

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(context.getIntValue(i));

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numPeriods+r));

	auto result = p.serialSGS(order, zr, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListFixedOvertimeModel::addAdditionalData(LSExpression& obj) {
	for (int r = 0; r < p.numRes; r++) {
		zrVar[r] = model.intVar(0, p.zmax[r]);
		obj.addOperand(zrVar[r]);
	}
}

vector<int> ListFixedOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs), zr(p.numRes);

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(sol.getIntValue(zrVar[r]));

	return p.serialSGS(order, zr, true).first;
}

lsdouble ListDynamicOvertimeModel::SerialSGSZrtDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	Matrix<int> zrt(p.numRes, p.numPeriods);
	if (context.count() < p.numJobs + p.numRes * p.numPeriods) return numeric_limits<double>::lowest();

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(context.getIntValue(i));

	for (int r = 0; r < p.numRes; r++)
		for (int t = 0; t < p.numPeriods; t++)
			zrt(r,t) = static_cast<int>(context.getIntValue(p.numJobs + r * p.numPeriods + t));

	auto result = p.serialSGS(order, zrt, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListDynamicOvertimeModel::addAdditionalData(LSExpression& obj) {
	for (int r = 0; r < p.numRes; r++) {
		for (int t = 0; t < p.numPeriods; t++) {
			zrtVar(r,t) = model.intVar(0, p.zmax[r]);
			obj.addOperand(zrtVar(r,t));
		}
	}
}

vector<int> ListDynamicOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	Matrix<int> zrt(p.numRes, p.numPeriods);

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	for (int r = 0; r < p.numRes; r++)
		for (int t = 0; t < p.numPeriods; t++)
			zrt(r,t) = static_cast<int>(sol.getIntValue(zrtVar(r,t)));

	return p.serialSGS(order, zrt, true).first;
}

void ListTauModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		tauVar[i] = model.floatVar(0.0, 1.0);
		obj.addOperand(tauVar[i]);
	}
}

vector<int> ListTauModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		tau[i] = static_cast<float>(sol.getDoubleValue(tauVar[i]));
	}
	return p.serialSGSTimeWindowArbitrary(order, tau, true).first;
}

lsdouble ListTauDiscreteModel::SerialSGSIntegerFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		int beta = static_cast<int>(context.getIntValue(p.numJobs + i));
		tau[i] = (float)(static_cast<double>(beta) / static_cast<double>(IV_COUNT - 1));
	}
	auto result = p.serialSGSTimeWindowArbitrary(order, tau, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListTauDiscreteModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		tauVar[i] = model.intVar(0, IV_COUNT - 1);
		obj.addOperand(tauVar[i]);
	}
}

vector<int> ListTauDiscreteModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		tau[i] = (float)(static_cast<double>(sol.getIntValue(tauVar[i])) / static_cast<double>(IV_COUNT - 1));
	}
	return p.serialSGSTimeWindowArbitrary(order, tau, true).first;
}

//======================================================================================================================

pair<LSModel, Matrix<LSExpression>> buildModel(ProjectWithOvertime &p, LocalSolver &ls) {
	auto model = ls.getModel();
	auto dummyExpr = model.createConstant(0LL);
    auto sts = p.serialSGS(p.topOrder);

	// Decision variables
	Matrix<LSExpression> x(p.numJobs, p.numPeriods, [&](int j, int t) {
        return (t >= p.efts[j] && t <= p.lfts[j]) ? model.boolVar() : dummyExpr;
	});

	// Derived expressions
	Matrix<LSExpression> cumulatedDemand(p.numRes, p.numPeriods, [&](int r, int t) {
		LSExpression s = model.sum();
		p.eachJobConst([&](int j) {
			for (int tau = t; tau < t + p.durations[j]; tau++)
				if (tau >= p.efts[j] && tau <= p.lfts[j])
					s += p.demands(j, r) * x(j, tau);
		});
		return s;
	});

	vector<LSExpression> ft(p.numJobs);
	p.eachJobConst([&](int j) {
		auto s = model.sum();
		p.timeWindow(j, [&](int t) { s += t * x(j, t); });
		ft[j] = s;
	});

	// Objective function
	auto objfunc = model.sum();
	p.timeWindow(p.numJobs - 1, [&](int t) { objfunc += p.revenue[t] * x(p.numJobs - 1, t); });
	p.eachResPeriod([&](int r, int t) { objfunc += -p.kappa[r] * model.max(0, cumulatedDemand(r, t) - p.capacities[r]); });

	// Constraints

	// Each job once
	p.eachJob([&](int j) {
		auto xsum = model.sum();
		p.timeWindow(j, [&](int t) { xsum += x(j, t); });
		model.constraint(xsum == 1.0);
	});

	// Precedence restrictions
	p.eachJob([&](int j) {
		auto latestPredFt = model.max(0);
		p.eachJob([&](int i) {
			if (p.adjMx(i, j)) latestPredFt.addOperand(ft[i]);
		});
		model.constraint(ft[j] - p.durations[j] >= latestPredFt);
	});

	// Capacity restrictions
	p.eachResPeriodConst([&](int r, int t) {
		model.constraint(cumulatedDemand(r, t) <= model.sum(p.capacities[r], p.zmax[r]));
	});

	model.addObjective(objfunc, OD_Maximize);
	model.close();

    p.eachJobTimeWindow([&](int j, int t) {
        x(j, t).setValue(sts[j] == t - p.durations[j] ? 1LL : 0LL);
    });

	return make_pair(model, x);
}

vector<int> parseSolution(ProjectWithOvertime &p, Matrix<LSExpression> &x, LSSolution &sol) {
	vector<int> sts(p.numJobs);
	p.eachJobTimeWindow([&](int j, int t) {
		if (sol.getValue(x(j, t)) == 1)
			sts[j] = t - p.durations[j];
	});
	return sts;
}

//======================================================================================================================

class TraceCallback : public LSCallback {
    Utils::Tracer &tr;
    double secCtr;
public:
    TraceCallback(Utils::Tracer &_tr) : tr(_tr), secCtr(0.0) {}
    virtual void callback(LocalSolver &solver, LSCallbackType type) override;
    virtual ~TraceCallback();
};

void TraceCallback::callback(LocalSolver &solver, LSCallbackType type) {
    if(type == CT_Ticked) {
		secCtr += MSECS_BETWEEN_TRACES;
        lsdouble objval = solver.getModel().getObjective(0).getDoubleValue();
        tr.trace(secCtr, static_cast<float>(objval));
    }
}

TraceCallback::~TraceCallback() {
}

//======================================================================================================================

vector<int> LSSolver::solve(ProjectWithOvertime& p, double timeLimit, bool traceobj) {
	LocalSolver ls;
	Utils::Tracer tr("LocalSolverTrace");
	tr.trace(0.0, 0.0f);

	auto pair = buildModel(p, ls);
	auto model = pair.first;
	auto x = pair.second;

    TraceCallback cback(tr);
    ls.addCallback(CT_Ticked, &cback);

	ls.createPhase().setTimeLimit(static_cast<int>(timeLimit));
	auto param = ls.getParam();
	param.setNbThreads(1);
	param.setVerbosity(2);
    param.setTimeBetweenDisplays(static_cast<int>(MSECS_BETWEEN_TRACES/1000.0));
	ls.solve();

	auto sol = ls.getSolution();	
	if (sol.getStatus() != SS_Feasible) {
		//throw runtime_error("No feasible solution found!");
		vector<int> sts(p.numJobs, -1);
		return sts;
	}

	auto solvetime = ls.getStatistics().getRunningTime();
	cout << "Solvetime = " << solvetime << endl;

	return parseSolution(p, x, sol);
}

//======================================================================================================================

class RevenueFunction : public SchedulingNativeFunction {
public:
	explicit RevenueFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
	virtual lsdouble call(const LSNativeContext &context) override;
};

lsdouble RevenueFunction::call(const LSNativeContext &context) {
    return p.revenue[context.getIntValue(0)];
}

class CumulatedDemandFunction : public SchedulingNativeFunction {
public:
	explicit CumulatedDemandFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) { }
	virtual lsdouble call(const LSNativeContext &context) override;
};

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

//======================================================================================================================

vector<int> LSSolver::solveNative(ProjectWithOvertime &p) {
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
	param.setNbThreads(1);
	param.setVerbosity(2);
    ls.solve();

    auto sol = ls.getSolution();
    p.eachJob([&](int j) { sts[j] = static_cast<int>(sol.getIntValue(Sj[j])); });

    //auto status = sol.getStatus();
    //auto solvetime = ls.getStatistics().getRunningTime();

    return sts;
}

//======================================================================================================================

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
