
#include "OvertimeBoundModels.h"

using namespace std;
using namespace localsolver;

int ListFixedOvertimeModel::SerialSGSZrDecoder::varCount() { return p.numJobs + p.numRes; }

SGSResult ListFixedOvertimeModel::SerialSGSZrDecoder::decode(vector<int>& order, const LSNativeContext& context) {
	vector<int> zr(p.numRes);

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numJobs + r));

	auto res = p.serialSGSWithForwardBackwardImprovement(order, zr, !enforceTopOrdering);
	return res;
}

void ListFixedOvertimeModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	for (int r = 0; r < p.numRes; r++) {
		zrVar[r] = model.intVar(0, p.zmax[r]);
		obj.addOperand(zrVar[r]);
	}
}

vector<int> ListFixedOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs), zr(p.numRes);

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(sol.getIntValue(zrVar[r]));

	return p.serialSGSWithForwardBackwardImprovement(order, zr, !enforceTopOrdering).sts;
}

//==============================================================================================================

int ListDynamicOvertimeModel::SerialSGSZrtDecoder::varCount() {
	return p.numJobs + p.numRes * p.heuristicMakespanUpperBound();
}

SGSResult ListDynamicOvertimeModel::SerialSGSZrtDecoder::decode(vector<int>& order, const LSNativeContext& context) {
	int nperiods = p.heuristicMakespanUpperBound();
	Matrix<int> zrt(p.numRes, nperiods, [this, &context, nperiods](int r, int t) {
		return static_cast<int>(context.getIntValue(p.numJobs + r * nperiods + t));
	});

	auto res = p.serialSGSWithForwardBackwardImprovement(order, zrt, !enforceTopOrdering);
	return res;
}

void ListDynamicOvertimeModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	zrtVar.foreachAssign([this, &model, &obj](int r, int t) {
		auto v = model.intVar(0, p.zmax[r]);
		obj.addOperand(v);
		return v;
	});
}

vector<int> ListDynamicOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order = Utils::constructVector<int>(p.numJobs, [this, &sol](int i) {
		return static_cast<int>(sol.getIntValue(listElems[i]));
	});
	Matrix<int> zrt(p.numRes, p.heuristicMakespanUpperBound(), [this, &sol](int r, int t) {
		return static_cast<int>(sol.getIntValue(zrtVar(r, t)));
	});
	return p.serialSGSWithForwardBackwardImprovement(order, zrt, !enforceTopOrdering).sts;
}