#include "ListModel.h"


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
	if (model.getNbObjectives() == 0) {
		LSExpression obj = model.createExpression(O_Call, model.createNativeFunction(decoder));

		LSExpression activityList = model.listVar(p.numJobs);
		model.constraint(model.count(activityList) == p.numJobs);

		for (int i = 0; i < p.numJobs; i++) {
			listElems[i] = model.at(activityList, i);
			obj.addOperand(listElems[i]);
		}

		addAdditionalData(obj);

		model.addObjective(obj, OD_Maximize);
		model.close();
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