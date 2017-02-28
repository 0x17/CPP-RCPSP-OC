#include "ListModel.h"
#include "../Stopwatch.h"

lsdouble SchedulingNativeFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	if (context.count() < varCount()) return numeric_limits<double>::lowest();

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == Project::UNSCHEDULED)
			return numeric_limits<double>::lowest();
	}
	SGSResult result = decode(order, context);
	lsdouble profit = static_cast<lsdouble>(p.calcProfit(result));

	if (tr != nullptr) {
		if(profit > bks)
			bks = profit;
		tr->intervalTrace(static_cast<float>(bks));
	}

	// TODO: result.numSchedulesGenerated

	return profit;
}

SolverParams::SolverParams(double _tlimit, int _ilimit): BasicSolverParameters(_tlimit, _ilimit, false, "LocalSolverNative_", 1), seed(0), verbosityLevel(2), solverIx(0) {
}

string ListModel::traceFilenameForListModel(const string& outPath, int lsIndex, const string& instanceName) {
	return outPath + "LocalSolverNative" + to_string(lsIndex) + "Trace_" + instanceName;
}

ListModel::ListModel(ProjectWithOvertime& _p, SchedulingNativeFunction *_decoder) : p(_p), decoder(_decoder), listElems(_p.numJobs) {
}

ListModel::~ListModel() {
	if (decoder != nullptr)
		delete decoder;
}

vector<int> ListModel::solve(SolverParams params) {
    unique_ptr<Utils::Tracer> tr = nullptr;
	unique_ptr<TraceCallback> cback = nullptr;
	buildModel();
	applyParams(params);
    if(params.traceobj) {
        tr = make_unique<Utils::Tracer>(traceFilenameForListModel(params.outPath, params.solverIx, p.instanceName));
		cback = make_unique<TraceCallback>(*tr);
        //ls.addCallback(CT_TimeTicked, cback);
		decoder->setTracer(tr.get());
    }
	ls.solve();
	auto sol = ls.getSolution();
	return parseScheduleFromSolution(sol);
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

		addAdditionalData(model, obj);

		model.addObjective(obj, OD_Maximize);
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
		if(params.iterLimit != -1)
			phase.setIterationLimit(static_cast<long long>(params.iterLimit));
		if(params.timeLimit != -1.0)
			phase.setTimeLimit(static_cast<int>(params.timeLimit));
		auto param = ls.getParam();
		param.setNbThreads(params.threadCount);
		param.setSeed(params.seed);
		param.setVerbosity(params.verbosityLevel);
		if (params.traceobj) {
			int timeBetweenDisplays = static_cast<int>(ceil(MSECS_BETWEEN_TRACES_LONG / 1000.0));
			param.setTimeBetweenDisplays(timeBetweenDisplays);
		}
	}
}

TraceCallback::TraceCallback(Utils::Tracer &_tr) : tr(_tr), secCtr(0.0) {}

void TraceCallback::callback(LocalSolver &solver, LSCallbackType type) {
    if (type == CT_TimeTicked) {
        secCtr += MSECS_BETWEEN_TRACES_LONG;
        lsdouble objval = solver.getModel().getObjective(0).getDoubleValue();
        tr.trace(secCtr, static_cast<float>(objval));
    }
}

TraceCallback::~TraceCallback() {
}

