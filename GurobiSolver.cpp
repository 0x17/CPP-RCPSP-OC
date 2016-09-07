#include "GurobiSolver.h"
#include "ProjectWithOvertime.h"

#ifdef USE_GUROBI

#include <gurobi_c++.h>

GurobiSolver::GurobiSolver(ProjectWithOvertime &_p) :
	p(_p),
	env(GRBEnv()),
	model(GRBModel(env)),
	// x_{jt}, binary restriction
	xjt(p.numJobs, p.getHeuristicMaxMakespan()+1, [&](int j, int t) {
		return model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x" + to_string(j) + to_string(t));
	}),
	// z_{rt}, integer restriction, upper bound
	zrt(p.numRes, p.getHeuristicMaxMakespan()+1, [&](int r, int t) {
		return model.addVar(0.0, static_cast<double>(p.zmax[r]), 0.0, GRB_INTEGER, "z" + to_string(r) + to_string(t));
	})
{
	model.update();
	setupObjectiveFunction();
	setupConstraints();
}

void GurobiSolver::setupObjectiveFunction() {
	GRBLinExpr revenueForMakespan = 0;
	p.timeWindowBounded(p.numJobs - 1, [&](int t) {
		revenueForMakespan += p.revenue[t] * xjt(p.numJobs - 1, t);
	});
	GRBLinExpr overtimeCosts = 0;
	p.eachResPeriodBoundedConst([&](int r, int t) {
		overtimeCosts += p.kappa[r] * zrt(r, t);
	});

	model.setObjective(revenueForMakespan - overtimeCosts, GRB_MAXIMIZE);
}

void GurobiSolver::setupConstraints() {
	// each activity once
	p.eachJobConst([&](int j) {
		GRBLinExpr s = 0;
		p.timeWindowBounded(j, [&](int t) {
			s += xjt(j, t);
		});
		model.addConstr(s == 1, "eachOnce" + to_string(j));
	});

	// precedence
	p.eachJobPairConst([&](int i, int j) {
		if (p.adjMx(i, j)) {
			GRBLinExpr predFt = 0;
			p.timeWindowBounded(i, [&](int t) { predFt += xjt(i, t) * t; });

			GRBLinExpr succSt = 0;
			p.timeWindowBounded(j, [&](int t) { succSt += xjt(j, t) * t; });
			succSt -= p.durations[j];

			model.addConstr(predFt <= succSt, to_string(i) + " before " + to_string(j));
		}
	});

	// resource capacities
	p.eachResPeriodBoundedConst([&](int r, int t) {
		GRBLinExpr cumDemands = 0;
		p.eachJobConst([&](int j) {
			for (int tau = t; tau<min(t + p.durations[j], p.getHeuristicMaxMakespan() + 1); tau++) {
				cumDemands += p.demands(j, r) * xjt(j, tau);
			}
		});

		model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "res. restr. (r,t)=(" + to_string(r) + "," + to_string(t) + ")");
	});
}

vector<int> GurobiSolver::parseSchedule() const {
	vector<int> sts(p.numJobs);
	p.eachJobTimeWindowBounded([&](int j, int t) {
		if (xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
			sts[j] = t - p.durations[j];
		}
	});
	return sts;
}

vector<int> GurobiSolver::solve() {
	try {
		model.optimize();
		return parseSchedule();
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...) {
		cout << "Exception during optimization" << endl;
	}

	vector<int> sts(p.numJobs);
	return sts;
}
#else
vector<int> GurobiSolver::solve(ProjectWithOvertime& p) {
	vector<int> sts(p.numJobs);
	return sts;
}
#endif