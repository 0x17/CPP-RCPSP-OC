#pragma once

#include "ListModel.h"

class ListDeadlineModel : public ListModel {
	class QuasistableSGSFunction : public SchedulingNativeFunction {
		SGSResult fallbackResult;
	public:
		explicit QuasistableSGSFunction(ProjectWithOvertime &_p, SGSResult _fallbackResult) : SchedulingNativeFunction(_p), fallbackResult(_fallbackResult) {}
		int varCount() override;
		SGSResult decode(vector<int>& order, const LSNativeContext& context) override;
	};

	LSExpression deadlineVar;

	static SGSResult computeFallbackResult(Project& p);

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListDeadlineModel(ProjectWithOvertime &_p);
	virtual ~ListDeadlineModel() {}
};
