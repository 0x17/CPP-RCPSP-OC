
#include "FixedDeadlineModels.h"

using namespace std;
using namespace localsolver;

GSListModel::GSListModel(ProjectWithOvertime& _p, SchedulingNativeFunction *func) : ListModel(_p, func) {}
GSListModel::GSListModel(ProjectWithOvertime& _p) : ListModel(_p, new QuasistableSGSFunction(_p)) {}

int GSListModel::QuasistableSGSFunction::varCount() {
	return p.numJobs;
}

SGSResult GSListModel::QuasistableSGSFunction::decode(vector<int>& order, const LSNativeContext& context) {
	return p.goldenSectionSearchBasedOptimization(order, true);
}

void GSListModel::addAdditionalData(LSModel &model, LSExpression& obj) {}

vector<int> GSListModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
	return p.goldenSectionSearchBasedOptimization(order, true).sts;
}

//=====================================================================================================================


ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p, SchedulingNativeFunction *func) : ListModel(_p, func) {}
ListDeadlineModel::ListDeadlineModel(ProjectWithOvertime& _p) : ListModel(_p, new QuasistableSGSFunction(_p)) {}

int ListDeadlineModel::QuasistableSGSFunction::varCount() {
	return p.numJobs + 1;
}

SGSResult ListDeadlineModel::QuasistableSGSFunction::decode(vector<int>& order, const LSNativeContext& context) {
	int deadlineOffset = static_cast<int>(context.getIntValue(p.numJobs));
	return p.forwardBackwardDeadlineOffsetSGS(order, deadlineOffset, true);
}

void ListDeadlineModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	int lb = 0;
	int ub = p.makespan(p.serialSGS(p.topOrder)) - p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
	deadlineOffsetVar = model.intVar(lb, ub);
	obj.addOperand(deadlineOffsetVar);
}

vector<int> ListDeadlineModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	int deadlineOffset;

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	deadlineOffset = static_cast<int>(sol.getIntValue(deadlineOffsetVar));

	return p.forwardBackwardDeadlineOffsetSGS(order, deadlineOffset, true).sts;
}
