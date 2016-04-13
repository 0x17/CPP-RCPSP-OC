
#include "OvertimeBoundModels.h"

int ListFixedOvertimeModel::SerialSGSZrDecoder::varCount() { return p.numJobs + p.numRes; }

SGSResult ListFixedOvertimeModel::SerialSGSZrDecoder::decode(vector<int>& order, const LSNativeContext& context) {
	vector<int> zr(p.numRes);

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numJobs + r));

	auto res = p.serialSGS(order, zr, true);
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

	return p.serialSGS(order, zr, true).sts;
}

//==============================================================================================================

int ListDynamicOvertimeModel::SerialSGSZrtDecoder::varCount() {
	return p.numJobs + p.numRes * p.numPeriods;
}

SGSResult ListDynamicOvertimeModel::SerialSGSZrtDecoder::decode(vector<int>& order, const LSNativeContext& context) {
	Matrix<int> zrt(p.numRes, p.numPeriods);

	for (int r = 0; r < p.numRes; r++)
		for (int t = 0; t < p.numPeriods; t++)
			zrt(r, t) = static_cast<int>(context.getIntValue(p.numJobs + r * p.numPeriods + t));

	auto res = p.serialSGS(order, zrt, true);
	return res;
}

void ListDynamicOvertimeModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	for (int r = 0; r < p.numRes; r++) {
		for (int t = 0; t < p.numPeriods; t++) {
			zrtVar(r, t) = model.intVar(0, p.zmax[r]);
			obj.addOperand(zrtVar(r, t));
		}
	}
}

vector<int> ListDynamicOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	Matrix<int> zrt(p.numRes, p.numPeriods);

	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));

	for (int r = 0; r < p.numRes; r++)
		for (int t = 0; t < p.numPeriods; t++)
			zrt(r, t) = static_cast<int>(sol.getIntValue(zrtVar(r, t)));

	return p.serialSGS(order, zrt, true).sts;
}