#include "ListModel.h"

ListModel::ListModel(ProjectWithOvertime& _p) : p(_p), decoder(nullptr), listElems(_p.numJobs) {}

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
		decoder = genDecoder();
		auto nfunc = model.createNativeFunction(decoder);
		LSExpression obj = model.createExpression(O_Call, nfunc);

		LSExpression activityList = model.listVar(p.numJobs);
		model.constraint(model.count(activityList) == p.numJobs);

		for (int i = 0; i < p.numJobs; i++) {
			listElems[i] = model.at(activityList, i);
			obj.addOperand(listElems[i]);
		}

		addAdditionalData(model, obj);

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