#pragma once

#include "ListModel.h"

class ListFixedOvertimeModel : public ListModel {
	class SerialSGSZrDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrDecoder(ProjectWithOvertime &_p, bool _enforceTopOrdering) : SchedulingNativeFunction(_p, _enforceTopOrdering) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	std::vector<localsolver::LSExpression> zrVar;

public:
	explicit ListFixedOvertimeModel(ProjectWithOvertime &_p, bool _enforceTopOrdering = false) : ListModel(_p, new SerialSGSZrDecoder(_p, _enforceTopOrdering), _enforceTopOrdering), zrVar(p.numRes) {}
	~ListFixedOvertimeModel() final = default;
};

class ListDynamicOvertimeModel : public ListModel {
	class SerialSGSZrtDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrtDecoder(ProjectWithOvertime &_p, bool _enforceTopOrdering) : SchedulingNativeFunction(_p, _enforceTopOrdering) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	Matrix<localsolver::LSExpression> zrtVar;

public:
	explicit ListDynamicOvertimeModel(ProjectWithOvertime &_p, bool _enforceTopOrdering = false) : ListModel(_p, new SerialSGSZrtDecoder(_p, _enforceTopOrdering), _enforceTopOrdering), zrtVar(p.numRes, p.heuristicMakespanUpperBound()) {}
	~ListDynamicOvertimeModel() final = default;
};

//==============================================================================================================

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
	explicit RandomKeyDynamicOvertimeModel(ProjectWithOvertime &_p, bool _enforceTopOrdering = false) : RandomKeyModel(_p, new SerialSGSRandomKeyZrtDecoder(_p)), zrtVar(p.numRes, p.heuristicMakespanUpperBound()) {}
	~RandomKeyDynamicOvertimeModel() final = default;
};