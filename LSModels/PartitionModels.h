#pragma once

#ifndef DISABLE_LOCALSOLVER

#include "ListModel.h"

class PartitionsSchedulingNativeFunction : public BaseSchedulingNativeFunction {
public:
	explicit PartitionsSchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	~PartitionsSchedulingNativeFunction() override = default;
	SGSResult decode(const Matrix<int> &partitions, const localsolver::LSNativeContext &context);
	boost::optional<SGSResult> coreComputation(const localsolver::LSNativeContext &context) override;

	int varCount() override;
};

class PartitionsModel : public LSBaseModel {
protected:
	Matrix<localsolver::LSExpression> partitionElems;
public:
	explicit PartitionsModel(ProjectWithOvertime &_p);
	~PartitionsModel() override = default;

protected:
	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) override;

	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) override;

private:
	std::vector<localsolver::LSExpression> partitions;

	void buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) override;
	virtual void applyInitialSolution() override;
};

//=================================================================================================

class ActivityListPartitionsModel : public ListModel {
	class ActivityListPartitionsSchedulingNativeFunction : public ListSchedulingNativeFunction {
	public:
		explicit ActivityListPartitionsSchedulingNativeFunction(ProjectWithOvertime &_p) : ListSchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

protected:
	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

public:
	ActivityListPartitionsModel(ProjectWithOvertime &_p, ListSchedulingNativeFunction *func);
	ActivityListPartitionsModel(ProjectWithOvertime &_p);
	virtual ~ActivityListPartitionsModel() {}
};

#endif