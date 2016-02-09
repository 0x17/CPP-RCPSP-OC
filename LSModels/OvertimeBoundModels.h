#pragma once

#include "ListModel.h"

class ListFixedOvertimeModel : public ListModel {
	class SerialSGSZrDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	vector<LSExpression> zrVar;

public:
	ListFixedOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrDecoder(_p)), zrVar(p.numRes) {}
	virtual ~ListFixedOvertimeModel() {}
};

class ListDynamicOvertimeModel : public ListModel {
	class SerialSGSZrtDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrtDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	Matrix<LSExpression> zrtVar;

public:
	ListDynamicOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrtDecoder(_p)), zrtVar(p.numRes, p.numPeriods) {}
	virtual ~ListDynamicOvertimeModel() {}
};
