#ifndef DISABLE_GUROBI

#include "GurobiSolver.h"
#include "ProjectWithOvertime.h"

using namespace std;

GurobiSolverBase::Options::Options() :
	BasicSolverParameters(GRB_INFINITY, -1, false, "GurobiTrace_", 1), // 0
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

GurobiSolverBase::GurobiSolverBase(const ProjectWithOvertime &_p, Options _opts, bool _heuristicPeriodUB) :
	p(_p),
	opts(_opts),
	env(setupOptions(opts)),
	model(*env),
	// x_{jt}, binary restriction
	xjt(p.numJobs, (_heuristicPeriodUB ? p.getHeuristicMaxMakespan() : p.numPeriods)+1, [&](int j, int t) {
		double ub = (t >= p.efts[j] && t <= (_heuristicPeriodUB ? p.lftsBounded[j] : p.lfts[j])) ? 1.0 : 0.0;
		return model.addVar(0.0, ub, 0.0, GRB_BINARY, "xj" + to_string(j) + "t" + to_string(t));
	}),
	// z_{rt}, integer restriction, upper bound
	zrt(p.numRes, (_heuristicPeriodUB ? p.getHeuristicMaxMakespan() : p.numPeriods)+1, [&](int r, int t) {
		return model.addVar(0.0, static_cast<double>(p.zmax[r]), 0.0, GRB_INTEGER, "zr" + to_string(r) + "t" + to_string(t));
	}),
	cback(opts.traceobj ? new CustomCallback(opts.outPath, p.instanceName) : nullptr),
	heuristicPeriodUB(_heuristicPeriodUB)
{
	model.update();
	if(cback != nullptr)
		model.setCallback(cback.get());
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

	env->set(GRB_IntParam_OutputFlag, 1);

	//env->set(GRB_DoubleParam_NodeLimit, opts.iterLimit);
	//env->set(GRB_DoubleParam_Heuristics, 1.0);
	return env;
}

vector<int> GurobiSolverBase::parseSchedule() const {
	vector<int> sts(p.numJobs, Project::UNSCHEDULED);
	if(heuristicPeriodUB) {
		p.eachJobTimeWindowBounded([&](int j, int t) {
			if (xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
				sts[j] = t - p.durations[j];
			}
		});
	} else {
		p.eachJobTimeWindow([&](int j, int t) {
			if (xjt(j, t).get(GRB_DoubleAttr_X) == 1.0) {
				sts[j] = t - p.durations[j];
			}
		});
	}
	return sts;
}

GurobiSolverBase::Result GurobiSolverBase::solve() {
	try {
		model.optimize();

		if(model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
			LOG_W("Solution infeasible!");
		}

		auto sts = parseSchedule();
		if(p.isCompleteSchedule(sts) && cback.get() != nullptr)
			cback->manualCallback(p.calcProfit(sts));

		return { sts, model.get(GRB_IntAttr_Status) == GRB_OPTIMAL };
	}
	catch (GRBException &e) {
		LOG_W("Error code = " + to_string(e.getErrorCode()));
		LOG_W(e.getMessage());
	}
	catch (...) {
		LOG_W("Exception during optimization");
	}
	vector<int> sts(p.numJobs);
	return {sts, false};
}

string GurobiSolverBase::traceFilenameForInstance(const string& outPath, const string& instanceName) {
	return outPath + "GurobiTrace_" + instanceName;
}

//================================================================================================================================================

void GurobiSolver::setupObjectiveFunction() {
	assert(heuristicPeriodUB);

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
			p.demandInPeriodMIP(j, t, [&](int tau) {
				cumDemands += p.demands(j, r) * xjt(j, tau);
			});
		});

		model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "res. restr. (r,t)=(" + to_string(r) + "," + to_string(t) + ")");
	});
}

void GurobiSolver::setupFeasibleMipStart() {
	if (!opts.useSeedSol) return;

	// Here we could use a GA result for reference value computation
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

GurobiSolver::GurobiSolver(const ProjectWithOvertime &_p, GurobiSolverBase::Options _opts) : GurobiSolverBase(_p, _opts, true) {}

//================================================================================================================================================

GurobiSubprojectSolver::GurobiSubprojectSolver(const ProjectWithOvertime& _p, Options& _opts)
:	 GurobiSolverBase(_p, _opts, false),
	 eachOnceConstraints(_p.numJobs),
	 isms(Utils::constructVector<GRBVar>(p.numPeriods, [this](int t) {return model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "ismakespan"+to_string(t));  }))
{
	cout << "New object construction!" << endl;
	buildModel();
}

void GurobiSubprojectSolver::setupObjectiveFunction() {
	assert(!heuristicPeriodUB);

	GRBLinExpr revenueForMakespan = 0;
	p.eachPeriodConst([&](int t) {
		revenueForMakespan += p.revenueExtrapolated[t] * isms[t];
	});

	GRBLinExpr overtimeCosts = 0;
	p.eachResPeriodConst([&](int r, int t) {
		overtimeCosts += p.kappa[r] * zrt(r, t);
	});

	model.setObjective(revenueForMakespan - overtimeCosts, GRB_MAXIMIZE);
}

void GurobiSubprojectSolver::setupConstraints() {
	// Link makespan
	GRBLinExpr mssum = 0;
	p.eachPeriodConst([&](int t) {
		mssum += isms[t] * t;
	});

	p.eachJobConst([&](int j) {
		GRBLinExpr ftjsum = 0;
		p.timeWindow(j, [&](int t) {
			ftjsum += xjt(j, t) * t;
		});
		model.addConstr(ftjsum <= mssum, "linkMakespanj" + to_string(j));
	});

	// Choose one makespan
	GRBLinExpr mssumonce = 0;
	p.eachPeriodConst([&](int t) {
		mssumonce += isms[t];
	});
	model.addConstr(mssumonce == 1, "chooseOneMakespan");

	// each activity once
	p.eachJobConst([&](int j) {
		GRBLinExpr s = 0;
		p.timeWindow(j, [&](int t) {
			s += xjt(j, t);
		});
		eachOnceConstraints[j] = model.addConstr(s == 1, "eachOncej" + to_string(j));
	});

	// precedence
	p.eachJobPairConst([&](int i, int j) {
		if (p.adjMx(i, j)) {
			GRBLinExpr predFt = 0;
			p.timeWindow(i, [&](int t) { predFt += xjt(i, t) * t; });

			GRBLinExpr succFt = 0;
			p.timeWindow(j, [&](int t) { succFt += xjt(j, t) * t; });

			precedenceConstraints[make_pair(i,j)] = model.addConstr(succFt - predFt >= p.durations[j], "i" + to_string(i) + "beforej" + to_string(j));
		}
	});

	// resource capacities
	p.eachResPeriodConst([&](int r, int t) {
		GRBLinExpr cumDemands = 0;

		p.eachJobConst([&](int j) {
			p.demandInPeriodMIP(j, t, [&](int tau) {
				cumDemands += p.demands(j, r) * xjt(j, tau);
			});
		});

		model.addConstr(cumDemands <= p.capacities[r] + zrt(r, t), "capresr" + to_string(r) + "t" + to_string(t));
	});
}

void GurobiSubprojectSolver::setupFeasibleMipStart() {
}

void GurobiSubprojectSolver::setupModelForSubproject(const std::vector<int> &sts, const std::vector<int> &nextPartition, bool forceInitIgnoredJobs) {
	p.eachJobConst([&](int j) {
		if(sts[j] != Project::UNSCHEDULED) {
			p.eachPeriodConst([&](int t) {
				double val = sts[j] + p.durations[j] == t ? 1.0 : 0.0;
				xjt(j, t).set(GRB_DoubleAttr_LB, val);
				xjt(j, t).set(GRB_DoubleAttr_UB, val);
			});

			p.eachJobConst([&](int i) {
				if(p.adjMx(i,j))
					precedenceConstraints[make_pair(i,j)].set(GRB_DoubleAttr_RHS, -GRB_INFINITY);
				if(p.adjMx(j,i))
					precedenceConstraints[make_pair(j,i)].set(GRB_DoubleAttr_RHS, -GRB_INFINITY);
			});
		}
		else if(find(nextPartition.begin(), nextPartition.end(), j) != nextPartition.end()) {
			int latestPredFt = p.computeLastPredFinishingTimeForSts(sts, j);
			p.eachPeriodConst([&](int t) {
				double ub = (t >= max(p.efts[j], latestPredFt+p.durations[j]) && t <= p.lfts[j]) ? 1.0 : 0.0;
				xjt(j, t).set(GRB_DoubleAttr_LB, 0.0);
				xjt(j, t).set(GRB_DoubleAttr_UB, ub);
			});

			eachOnceConstraints[j].set(GRB_DoubleAttr_RHS, 1.0);

			p.eachJobConst([&](int i) {
				if(p.adjMx(i,j) && find(nextPartition.begin(), nextPartition.end(), i) != nextPartition.end()) {
					precedenceConstraints[make_pair(i, j)].set(GRB_DoubleAttr_RHS, p.durations[j]);
				}
			});
		} else if(forceInitIgnoredJobs) {
			p.eachPeriodConst([&](int t) {
				xjt(j, t).set(GRB_DoubleAttr_LB, 0.0);
				xjt(j, t).set(GRB_DoubleAttr_UB, 0.0);
			});
			eachOnceConstraints[j].set(GRB_DoubleAttr_RHS, 0.0);
			p.eachJobConst([&](int i) {
				if(p.adjMx(i,j))
					precedenceConstraints[make_pair(i,j)].set(GRB_DoubleAttr_RHS, -GRB_INFINITY);
				if(p.adjMx(j,i))
					precedenceConstraints[make_pair(j,i)].set(GRB_DoubleAttr_RHS, -GRB_INFINITY);
			});
		}
	});

	model.update();
	model.reset();
}

#endif
