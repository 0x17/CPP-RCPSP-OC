#ifndef DISABLE_LOCALSOLVER

#include "OvertimeBoundModels.h"

using namespace std;
using namespace localsolver;

int ListFixedOvertimeModel::SerialSGSZrDecoder::varCount() { return p.numJobs + p.numRes; }

SGSResult ListFixedOvertimeModel::SerialSGSZrDecoder::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	vector<int> zr(p.numRes);

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numJobs + r));

	return options.parallelSGS ? p.parallelSGSWithForwardBackwardImprovement(order, zr) : p.serialSGSWithForwardBackwardImprovement(order, zr, !options.enforceTopOrdering);
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

	return options.parallelSGS ? p.parallelSGSWithForwardBackwardImprovement(order, zr).sts : p.serialSGSWithForwardBackwardImprovement(order, zr, !options.enforceTopOrdering).sts;
}

//==============================================================================================================

int ListDynamicOvertimeModel::SerialSGSZrtDecoder::varCount() {
	return p.numJobs + p.numRes * p.heuristicMakespanUpperBound();
}

SGSResult ListDynamicOvertimeModel::SerialSGSZrtDecoder::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	int nperiods = p.heuristicMakespanUpperBound();
	Matrix<int> zrt(p.numRes, nperiods, [this, &context, nperiods](int r, int t) {
		return static_cast<int>(context.getIntValue(p.numJobs + r * nperiods + t));
	});

	return options.parallelSGS ? p.parallelSGSWithForwardBackwardImprovement(order, zrt) : p.serialSGSWithForwardBackwardImprovement(order, zrt, !options.enforceTopOrdering);
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
	return options.parallelSGS ? p.parallelSGSWithForwardBackwardImprovement(order, zrt).sts : p.serialSGSWithForwardBackwardImprovement(order, zrt, !options.enforceTopOrdering).sts;
}

//==============================================================================================================

int RandomKeyDynamicOvertimeModel::SerialSGSRandomKeyZrtDecoder::varCount() {
	return p.numJobs + p.numRes * p.heuristicMakespanUpperBound();
}

SGSResult RandomKeyDynamicOvertimeModel::SerialSGSRandomKeyZrtDecoder::decode(vector<float>& priorities, const LSExternalArgumentValues& context) {
	int nperiods = p.heuristicMakespanUpperBound();
	Matrix<int> zrt(p.numRes, nperiods, [this, &context, nperiods](int r, int t) {
		return static_cast<int>(context.getIntValue(p.numJobs + r * nperiods + t));
	});

	return p.serialSGSWithRandomKeyAndFBI(priorities, zrt);
}

void RandomKeyDynamicOvertimeModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	zrtVar.foreachAssign([this, &model, &obj](int r, int t) {
		auto v = model.intVar(0, p.zmax[r]);
		obj.addOperand(v);
		return v;
	});
}

vector<int> RandomKeyDynamicOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	const vector<float> priorities = Utils::constructVector<float>(p.numJobs, [this, &sol](int i) {
		return static_cast<float>(sol.getDoubleValue(prioritiesElems[i]));
	});
	const Matrix<int> zrt(p.numRes, p.heuristicMakespanUpperBound(), [this, &sol](int r, int t) {
		return static_cast<int>(sol.getIntValue(zrtVar(r, t)));
	});
	return p.serialSGSWithRandomKeyAndFBI(priorities, zrt).sts;
}

//==============================================================================================================


int RandomKeyFixedOvertimeModel::SerialSGSRandomKeyZrDecoder::varCount() { return p.numJobs + p.numRes; }

SGSResult RandomKeyFixedOvertimeModel::SerialSGSRandomKeyZrDecoder::decode(vector<float>& priorities, const LSExternalArgumentValues& context) {
	vector<int> zr(p.numRes);

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(context.getIntValue(p.numJobs + r));

	return p.serialSGSWithRandomKeyAndFBI(priorities, zr);
}

void RandomKeyFixedOvertimeModel::addAdditionalData(LSModel &model, LSExpression& obj) {
	for (int r = 0; r < p.numRes; r++) {
		zrVar[r] = model.intVar(0, p.zmax[r]);
		obj.addOperand(zrVar[r]);
	}
}

vector<int> RandomKeyFixedOvertimeModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<float> priorities(p.numJobs);
	vector<int> zr(p.numRes);

	for (int i = 0; i<p.numJobs; i++)
		priorities[i] = static_cast<float>(sol.getDoubleValue(prioritiesElems[i]));

	for (int r = 0; r < p.numRes; r++)
		zr[r] = static_cast<int>(sol.getIntValue(zrVar[r]));

	return p.serialSGSWithRandomKeyAndFBI(priorities, zr).sts;
}

#endif