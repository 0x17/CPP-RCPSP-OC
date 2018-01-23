#ifndef DISABLE_GUROBI

#include "GurobiSolver.h"
#include "ProjectWithOvertime.h"

using namespace std;

GurobiSolverBase::Options::Options() :
	BasicSolverParameters(GRB_INFINITY, -1, false, "GurobiTrace_", 0),
	useSeedSol(true),
	gap(0.0),
	displayInterval(1) {
}

GurobiSolverBase::CustomCallback::CustomCallback(const string &outPath, const string &instanceName) :
		tr(traceFilenameForInstance(outPath, instanceName)) {
	sw.start();
}

void GurobiSolverBase::CustomCallback::callback() {
	if (where == GRB_CB_MIP) {
		/*getDoubleInfo(GRB_CB_RUNTIME)*/
		tr.trace(sw.look(), static_cast<float>(getDoubleInfo(GRB_CB_MIP_OBJBST)), getDoubleInfo(GRB_CB_MIP_ITRCNT), getDoubleInfo(GRB_CB_MIP_ITRCNT));
	}
}

void GurobiSolverBase::CustomCallback::manualCallback(float bks) {
	tr.trace(sw.look(), bks, getDoubleInfo(GRB_CB_MIP_ITRCNT), getDoubleInfo(GRB_CB_MIP_ITRCNT));
}

GurobiSolverBase::Result::Result(vector<int> &_sts, bool _optimal): sts(_sts), optimal(_optimal) {}

GurobiSolverBase::GurobiSolverBase(const ProjectWithOvertime &_p, Options _opts) :
	p(_p),
	opts(_opts),
	env(setupOptions(opts)),
	model(*env),
	// x_{jt}, binary restriction
	xjt(p.numJobs, p.getHeuristicMaxMakespan()+1, [&](int j, int t) {
		double ub = (t >= p.efts[j] && t <= p.lfts[j]) ? 1.0 : 0.0;
		return model.addVar(0.0, ub, 0.0, GRB_BINARY, "xj" + to_string(j) + "t" + to_string(t));
	}),
	// z_{rt}, integer restriction, upper bound
	zrt(p.numRes, p.getHeuristicMaxMakespan()+1, [&](int r, int t) {
		return model.addVar(0.0, static_cast<double>(p.zmax[r]), 0.0, GRB_INTEGER, "zr" + to_string(r) + "t" + to_string(t));
	}),
	cback(opts.traceobj ? new CustomCallback(opts.outPath, p.instanceName) : nullptr)
{
//	model.update();
	if(cback != nullptr)
		model.setCallback(cback.get());
}

void GurobiSolverBase::restrictJobToTimeWindow(int j, int eft, int lft) {
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

void GurobiSolverBase::relaxJob(int j) {
	restrictJobToTimeWindow(j, p.efts[j], p.lfts[j]);
}

void GurobiSolverBase::relaxAllJobs() {
	p.eachJobConst([&](int j) { relaxJob(j); });
}

void GurobiSolverBase::fixJobsToPartialSchedule(const std::vector<int>& sts) {
	p.eachJobConst([&](int j) {
		if (sts[j] != Project::UNSCHEDULED) {
			p.eachPeriodBoundedConst([&](int t) {
				double val = sts[j] + p.durations[j] == t ? 1.0 : 0.0;
				xjt(j, t).set(GRB_DoubleAttr_LB, val);
				xjt(j, t).set(GRB_DoubleAttr_UB, val);
			});
		}
	});
}

void GurobiSolverBase::buildModel() {
	setupObjectiveFunction();
	setupConstraints();
	setupFeasibleMipStart();
	model.update();
}

unique_ptr<GRBEnv> GurobiSolverBase::setupOptions(Options opts) {
	unique_ptr<GRBEnv> env = make_unique<GRBEnv>();
	env->set(GRB_DoubleParam_MIPGap, opts.gap);
	env->set(GRB_DoubleParam_TimeLimit, opts.timeLimit);
	env->set(GRB_IntParam_DisplayInterval, opts.displayInterval);
	env->set(GRB_IntParam_Threads, opts.threadCount);
	env->set(GRB_IntParam_OutputFlag, 0);
	//env->set(GRB_DoubleParam_NodeLimit, opts.iterLimit);
	//env->set(GRB_DoubleParam_Heuristics, 1.0);
	return env;
}

vector<int> GurobiSolverBase::parseSchedule() const {
	//LOG_I("Parsing schedule from decision variable values");

	vector<int> sts(p.numJobs, Project::UNSCHEDULED);
	p.eachJobTimeWindowBounded([&](int j, int t) {
		if (xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
			sts[j] = t - p.durations[j];
		}
	});
	return sts;
}

GurobiSolverBase::Result GurobiSolverBase::solve() {
	//LOG_I("Optimizing model");

	try {
		model.optimize();
		auto sts = parseSchedule();
		if(p.isCompleteSchedule(sts) && cback.get() != nullptr)
			cback->manualCallback(p.calcProfit(sts));
		return { sts, model.get(GRB_IntAttr_Status) == GRB_OPTIMAL };
	}
	catch (GRBException &e) {
		LOG_I("Error code = " + to_string(e.getErrorCode()));
		LOG_I(e.getMessage());
	}
	catch (...) {
		LOG_I("Exception during optimization");
	}
	vector<int> sts(p.numJobs);
	return {sts, false};
}

string GurobiSolverBase::traceFilenameForInstance(const string& outPath, const string& instanceName) {
	return outPath + "GurobiTrace_" + instanceName;
}

//================================================================================================================================================

void GurobiSolver::setupObjectiveFunction() {
	//LOG_I("Setting up objective function");

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
	//LOG_I("Setting up constraints");

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

	//LOG_I("Seeding mip solver with feasible heuristic start solution (serial sgs schedule)");

	auto sts = p.serialSGS(p.topOrder);

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
			cumDemand += sts[j] + 1 < t && t <= sts[j] + p.durations[j] ? p.demands(j, r) : 0;
		});
		zrt(r, t).set(GRB_DoubleAttr_Start, std::max(0, cumDemand - p.capacities[r]));
	});
}

//================================================================================================================================================

GurobiSubprojectSolver::GurobiSubprojectSolver(const ProjectWithOvertime& _p, Options& _opts, const std::vector<int>& _sts, const std::vector<int>& _nextPartition)
:	GurobiSolverBase(_p, _opts),
	sts(_sts),
	nextPartition(_nextPartition),
	isms(Utils::constructVector<GRBVar>(_p.heuristicMakespanUpperBound(), [this](int t) {return model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "ismakespan"+to_string(t));  })),
	jobsInSubproject(nextPartition)
{
	fixJobsToPartialSchedule(sts);

	for (int j = 0; j < sts.size(); j++) {
		if (sts[j] != Project::UNSCHEDULED)
			jobsInSubproject.push_back(j);
		else if(find(nextPartition.begin(), nextPartition.end(), j) == nextPartition.end()) {
			p.eachPeriodBoundedConst([&](int t) {
				xjt(j, t).set(GRB_DoubleAttr_LB, 0.0);
				xjt(j, t).set(GRB_DoubleAttr_UB, 0.0);
			});			
		}
	}
}

void GurobiSubprojectSolver::setupObjectiveFunction() {
	GRBLinExpr revenueForMakespan = 0;
	p.eachPeriodBoundedConst([&](int t) {
		revenueForMakespan += p.revenueExtrapolated[t] * isms[t];
	});
	GRBLinExpr overtimeCosts = 0;
	p.eachResPeriodBoundedConst([&](int r, int t) {
		overtimeCosts += p.kappa[r] * zrt(r, t);
	});
	model.setObjective(revenueForMakespan - overtimeCosts, GRB_MAXIMIZE);
}

void GurobiSubprojectSolver::setupConstraints() {
	// Link makespan
	GRBLinExpr mssum = 0;
	p.eachPeriodBoundedConst([&](int t) {
		mssum += isms[t] * t;
	});

	for (int j : jobsInSubproject) {
		GRBLinExpr ftjsum = 0;
		p.timeWindowBounded(j, [&](int t) {
			ftjsum += xjt(j, t) * t;
		});
		model.addConstr(ftjsum <= mssum, "linkMakespanj" + to_string(j));
	}

	// Choose one makespan
	GRBLinExpr mssumonce = 0;
	p.eachPeriodBoundedConst([&](int t) {
		mssumonce += isms[t];
	});
	model.addConstr(mssumonce == 1, "chooseOneMakespan");

	// each activity once
	for(int j : nextPartition) {
		GRBLinExpr s = 0;
		p.timeWindowBounded(j, [&](int t) {
			s += xjt(j, t);
		});
		model.addConstr(s == 1, "eachOncej" + to_string(j));
	}

	// precedence
	for (int i : jobsInSubproject) {
		for (int j : jobsInSubproject) {
			if (p.adjMx(i, j)) {
				GRBLinExpr predFt = 0;
				p.timeWindowBounded(i, [&](int t) { predFt += xjt(i, t) * t; });

				GRBLinExpr succSt = 0;
				p.timeWindowBounded(j, [&](int t) { succSt += xjt(j, t) * t; });
				succSt -= p.durations[j];

				model.addConstr(predFt <= succSt, "i" + to_string(i) + "beforej" + to_string(j));
			}
		}
	}

	// resource capacities
	p.eachResPeriodBoundedConst([&](int r, int t) {
		GRBLinExpr cumDemands = 0;

		for(int j : jobsInSubproject) {
			p.demandInPeriodMIP(j, t, [&](int tau) {
				cumDemands += p.demands(j, r) * xjt(j, tau);
			});
		}

		model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "capresr" + to_string(r) + "t" + to_string(t));
	});
}

void GurobiSubprojectSolver::setupFeasibleMipStart() {
}

#endif
