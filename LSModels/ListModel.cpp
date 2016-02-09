#include "ListModel.h"

lsdouble SchedulingNativeFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	if (context.count() < varCount()) return numeric_limits<double>::lowest();

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == -1)
			return numeric_limits<double>::lowest();
	}

	SGSResult result = decode(order, context);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

ListModel::ListModel(ProjectWithOvertime& _p, SchedulingNativeFunction *_decoder) : p(_p), decoder(_decoder), listElems(_p.numJobs) {
}

ListModel::~ListModel() {
	if (decoder != nullptr)
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
	if (ls.getNbPhases() == 0) {
		ls.createPhase().setTimeLimit(static_cast<int>(params.timeLimit));
		auto param = ls.getParam();
		param.setNbThreads(params.threadCount);
		param.setSeed(params.seed);
		param.setVerbosity(params.verbosityLevel);
	}
}