
#include "OvertimeBoundModels.h"

lsdouble ListFixedOvertimeModel::SerialSGSZrDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs), zr(p.numRes);
	if (context.count() < p.numJobs + p.numRes) return numeric_limits<double>::lowest();

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == -1)
			return numeric_limits<double>::lowest();
	}

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numPeriods + r));

	auto result = p.serialSGS(order, zr, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
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

	return p.serialSGS(order, zr, true).first;
}

//==============================================================================================================

lsdouble ListDynamicOvertimeModel::SerialSGSZrtDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	Matrix<int> zrt(p.numRes, p.numPeriods);
	if (context.count() < p.numJobs + p.numRes * p.numPeriods) return numeric_limits<double>::lowest();

	for (int i = 0; i < p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		if (order[i] == -1)
			return numeric_limits<double>::lowest();
	}

	for (int r = 0; r < p.numRes; r++)
		for (int t = 0; t < p.numPeriods; t++)
			zrt(r, t) = static_cast<int>(context.getIntValue(p.numJobs + r * p.numPeriods + t));

	auto result = p.serialSGS(order, zrt, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
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

	return p.serialSGS(order, zrt, true).first;
}