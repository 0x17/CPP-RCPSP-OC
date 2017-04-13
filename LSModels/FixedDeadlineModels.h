#pragma once

#include "ListModel.h"

class GSListModel : public ListModel {
	class QuasistableSGSFunction : public SchedulingNativeFunction {
	public:
		explicit QuasistableSGSFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(vector<int>& order, const LSNativeContext& context) override;
	};

protected:
	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	GSListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *func);
	GSListModel(ProjectWithOvertime &_p);
	virtual ~GSListModel() {}
};

class ListDeadlineModel : public ListModel {
	class QuasistableSGSFunction : public SchedulingNativeFunction {
	public:
		explicit QuasistableSGSFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(vector<int>& order, const LSNativeContext& context) override;
	};

protected:
	LSExpression deadlineOffsetVar;

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListDeadlineModel(ProjectWithOvertime &_p, SchedulingNativeFunction *func);
	ListDeadlineModel(ProjectWithOvertime &_p);
	virtual ~ListDeadlineModel() {}
};
