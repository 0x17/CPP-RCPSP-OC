#include "ListModel.h"
#include "../Stopwatch.h"
#include "../Logger.h"

#include <boost/filesystem.hpp>

using namespace std;
using namespace localsolver;

#define DIRTY_ENFORCE_ITERLIM_HACK

#ifdef DIRTY_ENFORCE_ITERLIM_HACK
static LocalSolver *_ls = nullptr;
static int _nschedules = 0;
static int _nindividuals = 0;
static int _schedule_limit = -1;
#endif

LSModelOptions LSBaseModel::options;

void LSModelOptions::from_json(const json11::Json &obj) {
	map<string, bool *> keyNamesToBooleanSlots = {
		{"enforceTopOrdering", &enforceTopOrdering},
		{"parallelSGS", &parallelSGS}
	};

	map<string, int *> keyNamesToIntSlots = {
			{"partitionSize", &partitionSize}
	};

	JsonUtils::assignBooleanSlotsFromJsonWithMapping(obj, keyNamesToBooleanSlots);
	JsonUtils::assignNumberSlotsFromJsonWithMapping<int>(obj, keyNamesToIntSlots);
}

json11::Json LSModelOptions::to_json() const {
	return json11::Json::object {
		{"enforceTopOrdering", enforceTopOrdering},
		{"parallelSGS", parallelSGS},
		{"partitionSize", partitionSize}
	};
}

localsolver::lsdouble BaseSchedulingNativeFunction::call(const localsolver::LSNativeContext &context) {
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	if (_schedule_limit != -1 && _nschedules >= _schedule_limit && _ls != nullptr) {
		_ls->stop();
		return -1.0;
	}
#endif

	const boost::optional<SGSResult> result = coreComputation(context);
	if(!result) return numeric_limits<double>::lowest();

	const auto profit = static_cast<lsdouble>(p.calcProfit(result.get()));

#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	_nschedules += result.get().numSchedulesGenerated;
	_nindividuals++;
#endif

	if (tr != nullptr) {
		if (profit > bks)
			bks = profit;

		tr->intervalTrace(static_cast<float>(bks), _nschedules, _nindividuals);
		tr->countTrace(static_cast<float>(bks), _nschedules, _nindividuals);
	}

	return profit;
}

ProjectNativeFunction::ProjectNativeFunction(ProjectWithOvertime &_p) : p(_p) {}

void ProjectNativeFunction::setTracer(Utils::Tracer *tr) { this->tr = tr; }


boost::optional<SGSResult> ListSchedulingNativeFunction::coreComputation(const LSNativeContext& context) {
	vector<int> order(static_cast<unsigned long>(p.numJobs));
	if (context.count() < varCount()) return boost::optional<SGSResult>{};

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == Project::UNSCHEDULED)
			return boost::optional<SGSResult>{};
	}

	if (LSBaseModel::getOptions().enforceTopOrdering && !p.isOrderFeasible(order)) {
		return boost::optional<SGSResult>{};
	}

	return decode(order, context);

}

lsdouble TopOrderChecker::call(const LSNativeContext& context) {
	vector<int> order(static_cast<unsigned long>(p.numJobs));
	if(context.count() < order.size()) return numeric_limits<double>::lowest();
	for(int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if(order[i] == Project::UNSCHEDULED)
			return numeric_limits<double>::lowest();
	}
	return p.isOrderFeasible(order) ? 1.0 : 0.0;

}

string LSBaseModel::traceFilenameForLSModel(const string& outPath, int lsIndex, const string& instanceName) {
	return outPath + "LocalSolverNative" + to_string(lsIndex) + "Trace_" + instanceName;
}

ListModel::ListModel(ProjectWithOvertime& _p, ListSchedulingNativeFunction *_decoder) : LSBaseModel(_p, _decoder), listElems(_p.numJobs), topOrderChecker(ListModel::getOptions().enforceTopOrdering ? new TopOrderChecker(_p) : nullptr) {
}

vector<int> LSBaseModel::solve(SolverParams params) {
	std::unique_ptr<Utils::Tracer> tr = nullptr;
	std::unique_ptr<TraceCallback> cback = nullptr;
	buildBaseModel();
	applyParams(params);
    if(params.traceobj) {
        tr = std::make_unique<Utils::Tracer>(traceFilenameForLSModel(params.outPath, params.solverIx, p.instanceName));
		cback = std::make_unique<TraceCallback>(*tr);
        //ls.addCallback(CT_TimeTicked, cback);
		decoder->setTracer(tr.get());
    }
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	if(tr != nullptr)
		tr->setTraceMode(Utils::Tracer::TraceMode::ONLY_COUNT);
	_nschedules = _nindividuals = 0;
#endif
	ls.solve();
	auto sol = ls.getSolution();
	assert(sol.getStatus() == LSSolutionStatus::SS_Feasible || sol.getStatus() == LSSolutionStatus::SS_Optimal);
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	LOG_I("Number of calls: " + to_string(_nschedules) + " vs. iteration limit = " + to_string(params.iterLimit));
#endif
	return parseScheduleFromSolution(sol);
}

template<class T>
LSExpression convertMatrixToRowMajorLocalSolverArray(LSModel &model, const Matrix<T>& mx) {
	auto lsarray = model.array();
	for (int i = 0; i<mx.getM(); i++) {
		for (int j = 0; j<mx.getN(); j++) {
			lsarray.addOperand(mx(i, j));
		}
	}
	return lsarray;
}

LSExpression accessRowMajorLocalSolverArray(LSModel &model, LSExpression &arr, int m, LSExpression &i, LSExpression &j) {
	return model.at(arr, i * m + j);
}

void ListModel::addTopologicalOrderingConstraint(LSModel &model, LSExpression &activityList) const {
	/*for(int i=0; i<p.numJobs; i++) {
		for(int j=0; j<p.numJobs; j++) {
			if(p.adjMx(i,j)) {
				model.constraint(model.indexOf(activityList, i) < model.indexOf(activityList, j));
			}
		}
	}*/
	const auto topOrderCheckerFunc = model.createNativeFunction(topOrderChecker.get());
	auto topOrderCheckerExpr = model.call(topOrderCheckerFunc);
	for(int i=0; i<p.numJobs; i++)
		topOrderCheckerExpr.addOperand(listElems[i]);
	model.constraint(topOrderCheckerExpr == 1.0);

	/*for(int ix = 0; ix < p.numJobs; ix++) {
		auto atIndex = model.at(activityList, ix);
		for(int bix = 0; bix < ix; bix++) {
			auto atBeforeIndex = model.at(activityList, bix);
			auto adjMxArray = convertMatrixToRowMajorLocalSolverArray<char>(model, p.adjMx);
			auto atIndexIsPredOfBeforeIndex = accessRowMajorLocalSolverArray(model, adjMxArray, p.numJobs, atIndex, atBeforeIndex);
			model.constraint(atIndexIsPredOfBeforeIndex == 0);
		}
	}*/
}

void ListModel::applyInitialSolution() {
	// initial solution is topological ordering of jobs
	auto coll = activityList.getCollectionValue();
	for (int i = 0; i < p.numJobs; i++)
		coll.add(p.topOrder[i]);
}

void LSBaseModel::buildBaseModel() {
	LSModel model = ls.getModel();
	if (model.getNbObjectives() == 0) {
		auto nfunc = model.createNativeFunction(decoder.get());
		LSExpression obj = model.call(nfunc);

		buildModel(model, obj);

		addAdditionalData(model, obj);

		model.maximize(obj);
		model.close();

		applyInitialSolution();
	}
}

void ListModel::buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) {
	activityList = model.listVar(p.numJobs);
	model.constraint(model.count(activityList) == p.numJobs);

	for (int i = 0; i < p.numJobs; i++) {
		listElems[i] = model.at(activityList, i);
		obj.addOperand(listElems[i]);
	}

	if(options.enforceTopOrdering)
		addTopologicalOrderingConstraint(model, activityList);
}

void LSBaseModel::applyParams(SolverParams &params) {
	if (ls.getNbPhases() == 0) {
		auto phase = ls.createPhase();
		if (params.iterLimit != -1) {
			phase.setIterationLimit(static_cast<long long>(params.iterLimit));
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
			_schedule_limit = params.iterLimit;
			_ls = &ls;
#endif
		}
		if(params.timeLimit != -1.0)
			phase.setTimeLimit(static_cast<int>(params.timeLimit));
		auto param = ls.getParam();
		param.setNbThreads(params.threadCount);
		param.setSeed(params.seed);
		param.setVerbosity(params.verbosityLevel);
		if (params.traceobj) {
			auto timeBetweenDisplays = static_cast<int>(ceil(MSECS_BETWEEN_TRACES_LONG / 1000.0));
			param.setTimeBetweenDisplays(timeBetweenDisplays);
		}
	}
}

TraceCallback::TraceCallback(Utils::Tracer &_tr) : tr(_tr), secCtr(0.0) {}

void TraceCallback::callback(LocalSolver &solver, LSCallbackType type) {
    if (type == CT_TimeTicked) {
        secCtr += MSECS_BETWEEN_TRACES_LONG;
        lsdouble objval = solver.getModel().getObjective(0).getDoubleValue();
        tr.trace(secCtr, static_cast<float>(objval), -1, -1);
    }
}

//======================================================================================================================

boost::optional<SGSResult> RandomKeySchedulingNativeFunction::coreComputation(const LSNativeContext& context) {
	vector<float> priorities(static_cast<unsigned long>(p.numJobs));
	if (context.count() < varCount()) return boost::optional<SGSResult> {};

	for (int i = 0; i < p.numJobs; i++) {
		priorities[i] = static_cast<float>(context.getDoubleValue(i));
	}
	return decode(priorities, context);
}

RandomKeyModel::RandomKeyModel(ProjectWithOvertime& _p, RandomKeySchedulingNativeFunction *_decoder) : LSBaseModel(_p, _decoder), prioritiesElems(_p.numJobs) {
}

void RandomKeyModel::buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) {
	for (int i = 0; i < p.numJobs; i++) {
		prioritiesElems[i] = model.floatVar(0.0, 1.0);
		obj.addOperand(prioritiesElems[i]);
	}
}

void RandomKeyModel::applyInitialSolution() {
	// initial solution: random key variant of topological ordering
	const vector<float> rk = p.activityListToRandomKey(p.topOrder);
	for (int i = 0; i < p.numJobs; i++) {
		prioritiesElems[i].setDoubleValue(rk[i]);
	}
}
