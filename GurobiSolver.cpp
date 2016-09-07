#include "GurobiSolver.h"
#include "ProjectWithOvertime.h"

#ifdef USE_GUROBI

#include <gurobi_c++.h>

vector<int> GurobiSolver::solve(ProjectWithOvertime& p) {
	vector<int> sts(p.numJobs);

	try {
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);

		int heuristicMaxMs = p.makespan(p.serialSGS(p.topOrder));

		// DECISION VARIABLES

		// x_{jt}, binary restriction
		Matrix<GRBVar> xjt(p.numJobs, p.numPeriods, [&](int j, int t) {
			double ub = (t >= p.efts[j] && t <= p.lfts[j]) ? 1.0 : 0.0;
			return model.addVar(0.0, ub, 0.0, GRB_BINARY, "x" + to_string(j) + to_string(t));
		});

		// z_{rt}, integer restriction, upper bound
		Matrix<GRBVar> zrt(p.numRes, p.numPeriods, [&](int r, int t) {
			double ub = (t <= heuristicMaxMs) ? static_cast<double>(p.zmax[r]) : 0.0;
			return model.addVar(0.0, ub, 0.0, GRB_INTEGER, "z" + to_string(r) + to_string(t));
		});

		model.update();

		// OBJECTIVE
		GRBLinExpr revenueForMakespan = 0;
		p.timeWindowBounded(p.numJobs - 1, [&](int t) {
			revenueForMakespan += p.revenue[t] * xjt(p.numJobs - 1, t);
		});
		GRBLinExpr overtimeCosts = 0;
		p.eachResPeriodBoundedConst([&](int r, int t) {
			overtimeCosts += p.kappa[r] * zrt(r, t);
		});

		model.setObjective(revenueForMakespan - overtimeCosts, GRB_MAXIMIZE);

		// CONSTRAINTS

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
				for(int tau = t; tau<min(t + p.durations[j], p.getHeuristicMaxMakespan()+1); tau++) {
					cumDemands += p.demands(j, r) * xjt(j, tau);
				}
			});

			model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "res. restr. (r,t)=(" + to_string(r) + "," + to_string(t) + ")");
		});

		model.optimize();

		p.eachJobTimeWindow([&](int j, int t) {
			if(xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
				sts[j] = t - p.durations[j];
			}
		});
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...) {
		cout << "Exception during optimization" << endl;
	}

	return sts;
}
#else
vector<int> GurobiSolver::solve(ProjectWithOvertime& p) {
	vector<int> sts(p.numJobs);
	return sts;
}
#endif