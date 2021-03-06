//
// Created by André Schnabel on 24.10.15.
//

#ifndef DISABLE_LOCALSOLVER

#include <list>
#include <fstream>
#include <iostream>

#include "NaiveModels.h"
#include "../Stopwatch.h"
#include "../Logger.h"
#include "ListModel.h"

using namespace std;
using namespace localsolver;

//======================================================================================================================

std::pair<LSModel, Matrix<LSExpression>> buildModel(ProjectWithOvertime &p, LocalSolver &ls) {
	auto model = ls.getModel();
	auto dummyExpr = model.createConstant(0LL);

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

	std::vector<LSExpression> ft(p.numJobs);
	p.eachJobConst([&](int j) {
		auto s = model.sum();
		p.timeWindow(j, [&](int t) { s += t * x(j, t); });
		ft[j] = s;
	});

	// Objective function
	auto objfunc = model.sum();
	p.timeWindow(p.numJobs - 1, [&](int t) { objfunc += x(p.numJobs - 1, t) * p.revenue[t]; });
	p.eachResPeriod([&](int r, int t) { objfunc += model.max(0, cumulatedDemand(r, t) - p.capacities[r]) * -p.kappa[r]; });

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

	return std::make_pair(model, x);
}

std::vector<int> parseSolution(ProjectWithOvertime &p, Matrix<LSExpression> &x, LSSolution &sol) {
	std::vector<int> sts(p.numJobs);
	p.eachJobTimeWindow([&](int j, int t) {
		if (sol.getValue(x(j, t)) == 1)
			sts[j] = t - p.durations[j];
	});
	return sts;
}

//======================================================================================================================

std::vector<int> LSSolver::solve(ProjectWithOvertime& p, double timeLimit, int iterLimit, bool traceobj, string outPath) {
	LocalSolver ls;
	Utils::Tracer tr(outPath + "LocalSolverTrace_" + p.instanceName);

	auto pair = buildModel(p, ls);
	auto model = pair.first;
	auto x = pair.second;

	auto sts = p.serialSGS(p.topOrder);
	p.eachJobTimeWindow([&](int j, int t) {
		x(j, t).setValue(sts[j] + p.durations[j] == t ? 1LL : 0LL);
	});

	TraceCallback cback(tr);
	ls.addCallback(CT_TimeTicked, &cback);

	auto phase = ls.createPhase();
	if(timeLimit != -1.0)
		phase.setTimeLimit(static_cast<int>(timeLimit));
	if (iterLimit != -1)
		phase.setIterationLimit(static_cast<long long>(iterLimit));

	auto param = ls.getParam();
	param.setNbThreads(1);
	param.setVerbosity(2);
	param.setTimeBetweenDisplays(static_cast<int>(MSECS_BETWEEN_TRACES_LONG / 1000.0));
	ls.solve();

	auto sol = ls.getSolution();
	if (sol.getStatus() == SS_Infeasible) {
		//throw runtime_error("No feasible solution found!");
		return p.emptySchedule();
	}

	auto solvetime = ls.getStatistics().getRunningTime();
	std::cout << "Solvetime = " << solvetime << std::endl;

	return parseSolution(p, x, sol);
}

//======================================================================================================================

class RevenueFunction : public ProjectNativeFunction {
public:
	explicit RevenueFunction(ProjectWithOvertime &_p) : ProjectNativeFunction(_p) {}
	lsdouble call(const LSExternalArgumentValues &context) override;
	int varCount() override;
};

lsdouble RevenueFunction::call(const LSExternalArgumentValues &context) {
	assert(context.count() == 1);
	return p.revenue[context.getIntValue(0)];
}

int RevenueFunction::varCount() {
	return 1;
}

class CumulatedDemandFunction : public ProjectNativeFunction {
public:
	explicit CumulatedDemandFunction(ProjectWithOvertime &_p) : ProjectNativeFunction(_p) { }
	lsdouble call(const LSExternalArgumentValues &context) override;
	int varCount() override;
};

lsdouble CumulatedDemandFunction::call(const LSExternalArgumentValues &context) {
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

int CumulatedDemandFunction::varCount() {
	return 2 + p.numJobs;
}

//======================================================================================================================

std::vector<int> LSSolver::solveNative(ProjectWithOvertime &p) {
	std::vector<int> sts(p.numJobs);

	LocalSolver ls;
	auto model = ls.getModel();

	RevenueFunction revFunc(p);
	LSExpression revFuncExpr = model.createExternalFunction(&revFunc);

	CumulatedDemandFunction cumDemFunc(p);
	LSExpression cumDemFuncExpr = model.createExternalFunction(&cumDemFunc);

	// Decision variables
	std::vector<LSExpression> Sj(p.numJobs);
	p.eachJob([&](int j) {Sj[j] = model.intVar(0, p.numPeriods - 1); });
	Matrix<LSExpression> zrt(p.numRes, p.numPeriods);
	p.eachResPeriod([&](int r, int t) { zrt(r, t) = model.intVar(0, p.zmax[r]); });

	// Objective function
	LSExpression objfunc = model.sum();
	LSExpression revenueExpr = model.createExpression(O_Call, revFuncExpr);
	revenueExpr.addOperand(Sj[p.numJobs - 1]);
	objfunc += revenueExpr;
	p.eachResPeriod([&](int r, int t) { objfunc += zrt(r, t) * -p.kappa[r]; });

	// Precedence restriction
	p.eachJob([&](int j) {
		LSExpression lastPred = model.max(0);
		for (int i = 0; i<p.numJobs; i++)
			if (p.adjMx(i, j)) lastPred.addOperand(Sj[i] + p.durations[i]);
		model.constraint(lastPred <= Sj[j]);
	});

	// Capacity restriction
	p.eachResPeriod([&](int r, int t) {
		LSExpression cumDemandExpr = model.createExpression(O_Call, cumDemFuncExpr);
		cumDemandExpr.addOperand(r);
		cumDemandExpr.addOperand(t);
		p.eachJob([&](int j) { cumDemandExpr.addOperand(Sj[j]); });
		model.constraint(cumDemandExpr <= p.capacities[r] + zrt(r, t));
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
		std::to_string(p.numJobs),
		std::to_string(p.numPeriods),
		std::to_string(p.numRes) };

	outLines.push_back("durations");
	p.eachJob([&](int j) { outLines.push_back(std::to_string(p.durations[j])); });
	outLines.push_back("demands (j,r)-matrix (row major order)");
	p.eachJob([&](int j) { p.eachRes([&](int r) { outLines.push_back(std::to_string(p.demands(j, r))); }); });
	outLines.push_back("adjacency matrix (row major order)");
	p.eachJobPair([&](int i, int j) { outLines.push_back(p.adjMx(i, j) ? "1" : "0"); });
	outLines.push_back("capacities");
	p.eachRes([&](int r) { outLines.push_back(std::to_string(p.capacities[r])); });

	outLines.push_back("earliest finishing times");
	p.eachJob([&](int j) { outLines.push_back(std::to_string(p.efts[j])); });
	outLines.push_back("latest finishing times");
	p.eachJob([&](int j) { outLines.push_back(std::to_string(p.lfts[j])); });

	outLines.push_back("revenue");
	for (int t = 0; t<p.numPeriods + 1; t++) {
		outLines.push_back(std::to_string(p.revenue[t]));
	}
	outLines.push_back("upper bound for overtime");
	p.eachRes([&](int r) { outLines.push_back(std::to_string(p.zmax[r])); });
	outLines.push_back("kappa");
	p.eachRes([&](int r) { outLines.push_back(std::to_string(p.kappa[r])); });

	std::ofstream f(outFilename);
	if (!f.is_open()) return;
	for (auto onum : outLines)
		f << onum << "\n";
	f.close();
}

//======================================================================================================================

const static int partitionSize = 4;

class SerialSubSGSFunction : public localsolver::LSExternalFunction<localsolver::lsdouble> {
	const ProjectWithOvertime &p;
public:
	explicit SerialSubSGSFunction(const ProjectWithOvertime &_p) : p(_p) {}
	~SerialSubSGSFunction() override = default;

	lsdouble call(const LSExternalArgumentValues &context) override {
		assert(context.count() == p.numJobs);

		const auto partitionList = Utils::constructVector<int>(p.numJobs, [&](int j) { return static_cast<int>(context.getIntValue(j)); });
		if (!p.isPartitionListFeasible(partitionList, partitionSize)) {
			return numeric_limits<double>::lowest();
		}
		return p.calcProfit(p.serialOptimalSubSGSWithPartitionList(partitionList));
	}
};

std::vector<int> LSSolver::solvePartitionListModel(ProjectWithOvertime &p) {
	LocalSolver ls;
	auto model = ls.getModel();

	SerialSubSGSFunction sgsFunc(p);
	LSExpression sgsFuncExpr = model.createExternalFunction(&sgsFunc);

	int partitionCount = static_cast<int>(ceil(p.numJobs / partitionSize));

	// Decision variables
	auto partitionList = Utils::constructVector<LSExpression>(p.numJobs, [&](int j) { return model.intVar(0, partitionCount-1); });
	
	// Objective function
	LSExpression objfunc = model.createExpression(O_Call, sgsFuncExpr);
	p.eachJobConst([&](int j) { objfunc.addOperand(partitionList[j]); });

	// Precedence restriction
	p.eachJobPairConst([&](int i, int j) {
		if(p.adjMx(i, j)) {
			model.constraint(partitionList[i] <= partitionList[j]);
		}
	});

	// Enforce partition size
	for (int pix = 0; pix < partitionCount-1; pix++) {
		LSExpression njobsInThisPartition = model.sum();
		p.eachJobConst([&](int j) {
			njobsInThisPartition += model.iif(partitionList[j] == pix, 1, 0);
		});
		model.constraint(njobsInThisPartition == partitionSize);
	}

	model.addObjective(objfunc, OD_Maximize);
	model.close();

	ls.createPhase().setTimeLimit(30);
	auto param = ls.getParam();
	param.setNbThreads(1);
	param.setVerbosity(2);
	ls.solve();

	auto sol = ls.getSolution();
	const auto partitionListResult = Utils::constructVector<int>(p.numJobs, [&](int j) { return static_cast<int>(sol.getIntValue(partitionList[j])); });

	auto status = sol.getStatus();
	auto solvetime = ls.getStatistics().getRunningTime();

	return p.serialOptimalSubSGSWithPartitionList(partitionListResult);
}

#endif