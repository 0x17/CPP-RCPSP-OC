#pragma once

#include "ListModel.h"

class ListFixedOvertimeModel : public ListModel {
	class SerialSGSZrDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	std::vector<localsolver::LSExpression> zrVar;

public:
	ListFixedOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrDecoder(_p)), zrVar(p.numRes) {}
	virtual ~ListFixedOvertimeModel() {}
};

class ListDynamicOvertimeModel : public ListModel {
	class SerialSGSZrtDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrtDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	Matrix<localsolver::LSExpression> zrtVar;

public:
	ListDynamicOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrtDecoder(_p)), zrtVar(p.numRes, p.heuristicMakespanUpperBound()) {}
	virtual ~ListDynamicOvertimeModel() {}
};
