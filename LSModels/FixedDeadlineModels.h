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

protected:
	LSExpression deadlineVar;

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	static SGSResult computeFallbackResult(Project& p);

public:
	ListDeadlineModel(ProjectWithOvertime &_p, SchedulingNativeFunction *func);
	ListDeadlineModel(ProjectWithOvertime &_p);
	virtual ~ListDeadlineModel() {}
};

class ListBetaDeadlineModel : public ListDeadlineModel {
	vector<LSExpression> betaVar;

	class QuasistableSGSBetaFunction : public SchedulingNativeFunction {
		SGSResult fallbackResult;
	public:
		explicit QuasistableSGSBetaFunction(ProjectWithOvertime& _p, SGSResult _fallbackResult);
		int varCount() override;
		SGSResult decode(vector<int>& order, const LSNativeContext& context) override;
	};

	void addAdditionalData(LSModel& model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListBetaDeadlineModel(ProjectWithOvertime &_p) : ListDeadlineModel(_p, new QuasistableSGSBetaFunction(_p, computeFallbackResult(_p))), betaVar(_p.numJobs) {}
	virtual ~ListBetaDeadlineModel() {}
};