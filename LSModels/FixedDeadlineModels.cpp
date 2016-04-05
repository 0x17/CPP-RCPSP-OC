
#include "FixedDeadlineModels.h"

ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p, SchedulingNativeFunction *func) : ListModel(_p, func) {}
ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p) : ListModel(_p, new QuasistableSGSFunction(_p, computeFallbackResult(_p))) {}

int ListDeadlineModel::QuasistableSGSFunction::varCount() {
	return p.numJobs + 1;
}

SGSResult ListDeadlineModel::QuasistableSGSFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadline = static_cast<int>(context.getIntValue(p.numJobs));
	auto res = p.serialSGSWithDeadlineEarly(deadline, order);
	if (res.first) return res.second;
	return fallbackResult;
}

SGSResult ListDeadlineModel::computeFallbackResult(Project& p) {
	auto fsts = p.serialSGS(p.topOrder);
	auto fresRem = p.resRemForPartial(fsts);
	return make_pair(fsts, fresRem);
}

void ListDeadlineModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	int lb = p.makespan(p.serialSGS(p.topOrder, p.zmax).first);
	int ub = p.makespan(p.serialSGS(p.topOrder));
	deadlineVar = model.intVar(lb, ub);
	obj.addOperand(deadlineVar);
}

vector<int> ListDeadlineModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	int deadline;

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	deadline = static_cast<int>(sol.getIntValue(deadlineVar));

	auto res = p.serialSGSWithDeadlineEarly(deadline, order);
	if (res.first) return res.second.first;
	return p.emptySchedule();
}

//==========================================================================================================================================================================

ListBetaDeadlineModel::QuasistableSGSBetaFunction::QuasistableSGSBetaFunction(ProjectWithOvertime& _p, SGSResult _fallbackResult) : SchedulingNativeFunction(_p), fallbackResult(_fallbackResult) {}

int ListBetaDeadlineModel::QuasistableSGSBetaFunction::varCount() { return p.numJobs * 2 + 1; }

SGSResult ListBetaDeadlineModel::QuasistableSGSBetaFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadline = static_cast<int>(context.getIntValue(p.numJobs));
	vector<int> beta(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		beta[i] = static_cast<int>(context.getIntValue(p.numJobs + i));
	}
	auto res = p.serialSGSWithDeadlineBeta(deadline, order, beta);
	if (res.first) return res.second;
	return fallbackResult;
}

void ListBetaDeadlineModel::addAdditionalData(LSModel& model, LSExpression& obj) {
	ListDeadlineModel::addAdditionalData(model, obj);
	for (int i = 0; i < p.numJobs; i++) {
		betaVar[i] = model.boolVar();
		obj.addOperand(betaVar[i]);
	}
}

vector<int> ListBetaDeadlineModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs), betaVarVals(p.numJobs);
	int deadline;

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	deadline = static_cast<int>(sol.getIntValue(deadlineVar));

	for (int i = 0; i<p.numJobs; i++) {
		betaVarVals[i] = static_cast<int>(sol.getIntValue(betaVar[i]));
	}

	auto res = p.serialSGSWithDeadlineBeta(deadline, order, betaVarVals);
	if (res.first) return res.second.first;
	return p.emptySchedule();
}

