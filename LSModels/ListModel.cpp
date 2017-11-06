#include "ListModel.h"
#include "../Stopwatch.h"
#include "../Logger.h"

using namespace std;
using namespace localsolver;

#define DIRTY_ENFORCE_ITERLIM_HACK

#ifdef DIRTY_ENFORCE_ITERLIM_HACK
static LocalSolver *_ls = nullptr;
static int _nschedules = 0;
static int _nindividuals = 0;
static int _schedule_limit = -1;
#endif

lsdouble SchedulingNativeFunction::call(const LSNativeContext& context) {
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	if (_schedule_limit != -1 && _nschedules >= _schedule_limit && _ls != nullptr) {
		_ls->stop();
		return -1.0;
	}
#endif

	vector<int> order(static_cast<unsigned long>(p.numJobs));
	if (context.count() < varCount()) return numeric_limits<double>::lowest();

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == Project::UNSCHEDULED)
			return numeric_limits<double>::lowest();
	}

	if (enforceTopOrdering && !p.isOrderFeasible(order)) {
		return numeric_limits<double>::lowest();
	}

	SGSResult result = decode(order, context);
	auto profit = static_cast<lsdouble>(p.calcProfit(result));

#ifdef DIRTY_ENFORCE_ITERLIM_HACK
	_nschedules += result.numSchedulesGenerated;
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

SolverParams::SolverParams(double _tlimit, int _ilimit): BasicSolverParameters(_tlimit, _ilimit, false, "LocalSolverNative_", 1), seed(0), verbosityLevel(2), solverIx(0) {
}

string ListModel::traceFilenameForListModel(const string& outPath, int lsIndex, const string& instanceName) {
	return outPath + "LocalSolverNative" + to_string(lsIndex) + "Trace_" + instanceName;
}

ListModel::ListModel(ProjectWithOvertime& _p, SchedulingNativeFunction *_decoder, bool _enforceTopOrdering) : p(_p), decoder(_decoder), listElems(_p.numJobs), enforceTopOrdering(_enforceTopOrdering), topOrderChecker(_enforceTopOrdering ? new TopOrderChecker(_p) : nullptr) {
}

ListModel::~ListModel() {
	if (decoder != nullptr)
		delete decoder;
	if (topOrderChecker != nullptr)
		delete topOrderChecker;
}

vector<int> ListModel::solve(SolverParams params) {
	std::unique_ptr<Utils::Tracer> tr = nullptr;
	std::unique_ptr<TraceCallback> cback = nullptr;
	buildModel();
	applyParams(params);
    if(params.traceobj) {
        tr = std::make_unique<Utils::Tracer>(traceFilenameForListModel(params.outPath, params.solverIx, p.instanceName));
		cback = std::make_unique<TraceCallback>(*tr);
        //ls.addCallback(CT_TimeTicked, cback);
		decoder->setTracer(tr.get());
    }
#ifdef DIRTY_ENFORCE_ITERLIM_HACK
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
	auto topOrderCheckerFunc = model.createNativeFunction(topOrderChecker);
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

void ListModel::buildModel() {	
	LOG_I("Building local solver list variable model");
	LSModel model = ls.getModel();
	if (model.getNbObjectives() == 0) {
		auto nfunc = model.createNativeFunction(decoder);
		LSExpression obj = model.call(nfunc);

		LSExpression activityList = model.listVar(p.numJobs);
		model.constraint(model.count(activityList) == p.numJobs);

		for (int i = 0; i < p.numJobs; i++) {
			listElems[i] = model.at(activityList, i);
			obj.addOperand(listElems[i]);
		}

		if(enforceTopOrdering)
			addTopologicalOrderingConstraint(model, activityList);

		addAdditionalData(model, obj);

		model.maximize(obj);
		model.close();

		// initial solution 0, 1, ..., njobs
		auto coll = activityList.getCollectionValue();
		for (int i = 0; i < p.numJobs; i++)
			coll.add(i);
	}
}

void ListModel::applyParams(SolverParams &params) {
	LOG_I("Applying custom parameters");
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
