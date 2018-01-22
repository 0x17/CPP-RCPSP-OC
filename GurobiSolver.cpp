#include "OvertimeChoiceProviders.h"
#ifndef DISABLE_GUROBI

#include "GurobiSolver.h"
#include "ProjectWithOvertime.h"

using namespace std;

GurobiSolver::Options::Options() :
	BasicSolverParameters(GRB_INFINITY, -1, false, "GurobiTrace_", 0),
	useSeedSol(true),
	gap(0.0),
	displayInterval(1) {
}

GurobiSolver::CustomCallback::CustomCallback(const string &outPath, const string &instanceName) :
		tr(traceFilenameForInstance(outPath, instanceName)) {
	sw.start();
}

void GurobiSolver::CustomCallback::callback() {
	if (where == GRB_CB_MIP) {
		/*getDoubleInfo(GRB_CB_RUNTIME)*/
		tr.trace(sw.look(), static_cast<float>(getDoubleInfo(GRB_CB_MIP_OBJBST)), getIntInfo(GRB_CB_MIP_ITRCNT), getIntInfo(GRB_CB_MIP_ITRCNT));
	}
}

void GurobiSolver::CustomCallback::manualCallback(float bks) {
	tr.trace(sw.look(), bks, getIntInfo(GRB_CB_MIP_ITRCNT), getIntInfo(GRB_CB_MIP_ITRCNT));
}

GurobiSolver::Result::Result(vector<int> &_sts, bool _optimal): sts(_sts), optimal(_optimal) {}

GurobiSolver::GurobiSolver(ProjectWithOvertime &_p, Options _opts) :
	p(_p),
	opts(_opts),
	env(setupOptions(opts)),
	model(GRBModel(env)),
	// x_{jt}, binary restriction
	xjt(p.numJobs, p.getHeuristicMaxMakespan()+1, [&](int j, int t) {
		double ub = (t >= p.efts[j] && t <= p.lfts[j]) ? 1.0 : 0.0;
		return model.addVar(0.0, ub, 0.0, GRB_BINARY, "x" + to_string(j) + to_string(t));
	}),
	// z_{rt}, integer restriction, upper bound
	zrt(p.numRes, p.getHeuristicMaxMakespan()+1, [&](int r, int t) {
		return model.addVar(0.0, static_cast<double>(p.zmax[r]), 0.0, GRB_INTEGER, "z" + to_string(r) + to_string(t));
	}),
	cback(opts.outPath, p.instanceName)
{
	model.update();
	model.setCallback(&cback);
	setupObjectiveFunction();
	setupConstraints();
	setupFeasibleMipStart();
}

void GurobiSolver::restrictJobToTimeWindow(int j, int eft, int lft) {
	p.eachPeriodBoundedConst([&](int t) {
		if(t >= eft && t <= lft) {
			xjt(j, t).set(GRB_DoubleAttr_LB, 0.0);
			xjt(j, t).set(GRB_DoubleAttr_UB, 1.0);
		} else {
			xjt(j, t).set(GRB_DoubleAttr_LB, 0.0);
			xjt(j, t).set(GRB_DoubleAttr_UB, 0.0);
		}
	});
}

void GurobiSolver::relaxJob(int j) {
	restrictJobToTimeWindow(j, p.efts[j], p.lfts[j]);
}

void GurobiSolver::relaxAllJobs() {
	p.eachJobConst([&](int j) { relaxJob(j); });
}

GRBEnv GurobiSolver::setupOptions(Options opts) {
	GRBEnv env;
	env.set(GRB_DoubleParam_MIPGap, opts.gap);
	env.set(GRB_DoubleParam_TimeLimit, opts.timeLimit);
	env.set(GRB_IntParam_DisplayInterval, opts.displayInterval);
	env.set(GRB_IntParam_Threads, opts.threadCount);
	//env.set(GRB_DoubleParam_NodeLimit, opts.iterLimit);
	//env.set(GRB_DoubleParam_Heuristics, 1.0);
	return env;
}

void GurobiSolver::setupObjectiveFunction() {
	LOG_I("Setting up objective function");

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
	LOG_I("Setting up constraints");

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
			p.demandInPeriodMIP(j, t, [&](int tau) {
				cumDemands += p.demands(j, r) * xjt(j, tau);
			});
		});

		model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "res. restr. (r,t)=(" + to_string(r) + "," + to_string(t) + ")");
	});
}

void GurobiSolver::setupFeasibleMipStart() {
	if (!opts.useSeedSol) return;

	LOG_I("Seeding mip solver with feasible heuristic start solution (serial sgs schedule)");

	auto sts = SerialScheduleGenerationScheme(p).constructSchedule(al(p.topOrder), ocnone()).sts;

	p.eachJobConst([&](int j) {
		int ft = sts[j] + p.durations[j];
		p.eachPeriodBoundedConst([&](int t) {
			double sval = (ft == t) ? 1.0 : 0.0;
			xjt(j, t).set(GRB_DoubleAttr_Start, sval);
		});
	});

	p.eachResPeriodBoundedConst([&](int r, int t) {
		int cumDemand = 0;
		p.eachJobConst([&](int j) {
			cumDemand += sts[j] + 1 < t && t <= sts[j] + p.durations[j] ? p.demands(j,r) : 0;
		});
		zrt(r, t).set(GRB_DoubleAttr_Start, std::max(0, cumDemand-p.capacities[r]));
	});
}

vector<int> GurobiSolver::parseSchedule() const {
	LOG_I("Parsing schedule from decision variable values");

	vector<int> sts(p.numJobs);
	p.eachJobTimeWindowBounded([&](int j, int t) {
		if (xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
			sts[j] = t - p.durations[j];
		}
	});
	return sts;
}

GurobiSolver::Result GurobiSolver::solve() {
	LOG_I("Optimizing model");

	try {
		model.optimize();
		auto sts = parseSchedule();
		cback.manualCallback(p.calcProfit(sts));
		return { sts, model.get(GRB_IntAttr_Status) == GRB_OPTIMAL };
	}
	catch (GRBException e) {
		LOG_I("Error code = " + to_string(e.getErrorCode()));
		LOG_I(e.getMessage());
	}
	catch (...) {
		LOG_I("Exception during optimization");
	}
	vector<int> sts(p.numJobs);
	return {sts, false};
}

string GurobiSolver::traceFilenameForInstance(const string& outPath, const string& instanceName) {
	return outPath + "GurobiTrace_" + instanceName;
}
#endif
