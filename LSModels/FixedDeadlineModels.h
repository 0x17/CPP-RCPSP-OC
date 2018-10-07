#pragma once

#ifndef DISABLE_LOCALSOLVER

#include "ListModel.h"

class GSListModel : public ListModel {
	class QuasistableSGSFunction : public ListSchedulingNativeFunction {
	public:
		explicit QuasistableSGSFunction(ProjectWithOvertime &_p) : ListSchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

protected:
	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

public:
	GSListModel(ProjectWithOvertime &_p, ListSchedulingNativeFunction *func);
	GSListModel(ProjectWithOvertime &_p);
	virtual ~GSListModel() {}
};

class ListDeadlineModel : public ListModel {
	class QuasistableSGSFunction : public ListSchedulingNativeFunction {
	public:
		explicit QuasistableSGSFunction(ProjectWithOvertime &_p) : ListSchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

protected:
	localsolver::LSExpression deadlineOffsetVar;

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

public:
	ListDeadlineModel(ProjectWithOvertime &_p, ListSchedulingNativeFunction *func);
	ListDeadlineModel(ProjectWithOvertime &_p);
	virtual ~ListDeadlineModel() {}
};

#endif