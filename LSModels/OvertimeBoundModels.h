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
	explicit ListFixedOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrDecoder(_p)), zrVar(p.numRes) {}
	~ListFixedOvertimeModel() final = default;
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
	explicit ListDynamicOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrtDecoder(_p)), zrtVar(p.numRes, p.heuristicMakespanUpperBound()) {}
	~ListDynamicOvertimeModel() final = default;
};

//==============================================================================================================

class RandomKeyFixedOvertimeModel : public RandomKeyModel {
	class SerialSGSRandomKeyZrDecoder : public RandomKeySchedulingNativeFunction {
	public:
		explicit SerialSGSRandomKeyZrDecoder(ProjectWithOvertime &_p) : RandomKeySchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<float>& priorities, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	std::vector<localsolver::LSExpression> zrVar;

public:
	explicit RandomKeyFixedOvertimeModel(ProjectWithOvertime &_p) : RandomKeyModel(_p, new SerialSGSRandomKeyZrDecoder(_p)), zrVar(p.numRes) {}
	~RandomKeyFixedOvertimeModel() final = default;
};

class RandomKeyDynamicOvertimeModel : public RandomKeyModel {
	class SerialSGSRandomKeyZrtDecoder : public RandomKeySchedulingNativeFunction {
	public:
		explicit SerialSGSRandomKeyZrtDecoder(ProjectWithOvertime &_p) : RandomKeySchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<float>& priorities, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	Matrix<localsolver::LSExpression> zrtVar;

public:
	explicit RandomKeyDynamicOvertimeModel(ProjectWithOvertime &_p) : RandomKeyModel(_p, new SerialSGSRandomKeyZrtDecoder(_p)), zrtVar(p.numRes, p.heuristicMakespanUpperBound()) {}
	~RandomKeyDynamicOvertimeModel() final = default;
};