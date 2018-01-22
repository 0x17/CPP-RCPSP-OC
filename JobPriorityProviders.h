//
// Created by André Schnabel on 21.01.18.
//

#pragma once

#include <vector>

#include "Matrix.h"

class Project;

class IJobPrioProvider {
public:
	virtual int chooseEligibleWithLowestPriority(const Project &p, const std::vector<int> &sts) const = 0;
	virtual int chooseEligibleWithLowestPriority(const Project &p, const std::vector<bool> &unscheduled) const = 0;

	virtual int chooseEligibleWithHighestPriority(const Project &p, const std::vector<int> &sts) const = 0;
	virtual int chooseEligibleWithHighestPriority(const Project &p, const std::vector<bool> &unscheduled) const = 0;

	virtual int chooseEligibleWithHighestPrioAndResFeasInDt(const Project &p, const std::vector<int> &sts, const std::vector<int> &resRem) const = 0;
	virtual int chooseEligibleWithHighestPrioAndResFeasRuntime(const Project &p, int dt, const std::vector<int> &sts, const Matrix<int> &resRem) const = 0;

	virtual int getJobAtIndex(int k) const = 0;
};

class ActivityListPrioProvider : public IJobPrioProvider {
	const std::vector<int> &order;

public:
	ActivityListPrioProvider(const std::vector<int> &_order);

	int chooseEligibleWithLowestPriority(const Project &p, const std::vector<int> &sts) const override;
	int chooseEligibleWithLowestPriority(const Project &p, const std::vector<bool> &unscheduled) const override;
	int chooseEligibleWithHighestPriority(const Project &p, const std::vector<int> &sts) const override;
	int chooseEligibleWithHighestPriority(const Project &p, const std::vector<bool> &unscheduled) const override;
	int chooseEligibleWithHighestPrioAndResFeasInDt(const Project &p, const std::vector<int> &sts, const std::vector<int> &resRem) const override;
	int chooseEligibleWithHighestPrioAndResFeasRuntime(const Project &p, int dt, const std::vector<int> &sts, const Matrix<int> &resRem) const override;

	int getJobAtIndex(int k) const override;
};

class RandomKeyPrioProvider : public IJobPrioProvider {
	const std::vector<float> &priorities;

public:
	RandomKeyPrioProvider(const std::vector<float> &_priorities);

	int chooseEligibleWithLowestPriority(const Project &p, const std::vector<int> &sts) const override;
	int chooseEligibleWithLowestPriority(const Project &p, const std::vector<bool> &unscheduled) const override;
	int chooseEligibleWithHighestPriority(const Project &p, const std::vector<int> &sts) const override;
	int chooseEligibleWithHighestPriority(const Project &p, const std::vector<bool> &unscheduled) const override;
	int chooseEligibleWithHighestPrioAndResFeasInDt(const Project &p, const std::vector<int> &sts, const std::vector<int> &resRem) const override;
	int chooseEligibleWithHighestPrioAndResFeasRuntime(const Project &p, int dt, const std::vector<int> &sts, const Matrix<int> &resRem) const override;

	int getJobAtIndex(int k) const override;
};

inline ActivityListPrioProvider al(const std::vector<int> &order) { return order; }
inline RandomKeyPrioProvider rk(const std::vector<float> &priorities) { return priorities; }
