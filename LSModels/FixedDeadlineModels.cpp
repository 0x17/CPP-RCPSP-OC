
#include "FixedDeadlineModels.h"

ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p, SchedulingNativeFunction *func) : ListModel(_p, func) {}
ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p) : ListModel(_p, new QuasistableSGSFunction(_p, computeFallbackResult(_p))) {}

int ListDeadlineModel::QuasistableSGSFunction::varCount() {
	return p.numJobs + 1;
}

SGSResult ListDeadlineModel::QuasistableSGSFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadline = static_cast<int>(context.getIntValue(p.numJobs));
	//auto res = p.serialSGSWithDeadlineEarly(deadline, order);
	//if (res.valid) return static_cast<SGSResult>(res);
	//return fallbackResult;
	return p.earlyOvertimeDeadlineSGS(order, deadline, true);
}

SGSResult ListDeadlineModel::computeFallbackResult(Project& p) {
	auto fsts = p.serialSGS(p.topOrder);
	auto fresRem = p.resRemForPartial(fsts);
	return{ fsts, fresRem };
}

void ListDeadlineModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	int lb = p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
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
	if (res.valid) return res.sts;
	return computeFallbackResult(p).sts;
}

//==========================================================================================================================================================================

ListBetaDeadlineModel::QuasistableSGSBetaFunction::QuasistableSGSBetaFunction(ProjectWithOvertime& _p, SGSResult _fallbackResult) : SchedulingNativeFunction(_p), fallbackResult(_fallbackResult) {}

int ListBetaDeadlineModel::QuasistableSGSBetaFunction::varCount() { return p.numJobs * 2 + 1; }

SGSResult ListBetaDeadlineModel::QuasistableSGSBetaFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadline = static_cast<int>(context.getIntValue(p.numJobs));
	vector<int> beta(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		beta[i] = static_cast<int>(context.getIntValue(p.numJobs + 1 + i));
	}
	auto res = p.serialSGSWithDeadlineBeta(deadline, order, beta);

	if (res.valid) return static_cast<SGSResult>(res);
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
	if (res.valid) return res.sts;
	return computeFallbackResult(p).sts;
}

//==========================================================================================================================================================================

ListTauDeadlineModel::QuasistableSGSTauFunction::QuasistableSGSTauFunction(ProjectWithOvertime& _p, SGSResult _fallbackResult) : SchedulingNativeFunction(_p), fallbackResult(_fallbackResult) {}

int ListTauDeadlineModel::QuasistableSGSTauFunction::varCount() { return p.numJobs * 2 + 1; }

SGSResult ListTauDeadlineModel::QuasistableSGSTauFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadline = static_cast<int>(context.getIntValue(p.numJobs));
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		tau[i] = static_cast<float>(context.getDoubleValue(p.numJobs+1+i));
	}
	auto res = p.serialSGSWithDeadlineTau(deadline, order, tau);
	if (res.valid) return static_cast<SGSResult>(res);
	return fallbackResult;
}

void ListTauDeadlineModel::addAdditionalData(LSModel& model, LSExpression& obj) {
	ListDeadlineModel::addAdditionalData(model, obj);
	for (int i = 0; i < p.numJobs; i++) {
		tauVar[i] = model.floatVar(0.0, 1.0);
		obj.addOperand(tauVar[i]);
	}
}

vector<int> ListTauDeadlineModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	vector<float> tauVarVals(p.numJobs);
	int deadline;

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	deadline = static_cast<int>(sol.getIntValue(deadlineVar));

	for (int i = 0; i<p.numJobs; i++) {
		tauVarVals[i] = static_cast<float>(sol.getDoubleValue(tauVar[i]));
	}

	auto res = p.serialSGSWithDeadlineTau(deadline, order, tauVarVals);
	if (res.valid) return res.sts;
	return computeFallbackResult(p).sts;
}

