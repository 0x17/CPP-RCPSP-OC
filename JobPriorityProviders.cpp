//
// Created by André Schnabel on 21.01.18.
//

#include "JobPriorityProviders.h"
#include "Project.h"

bool ActivityListPrioProvider::robust = false;

ActivityListPrioProvider::ActivityListPrioProvider(const std::vector<int> &_order) : order(_order) {}

int ActivityListPrioProvider::chooseEligibleWithLowestPriority(const Project &p, const std::vector<int> &sts, int schedulingStep) const {
	if(!robust) return getJobAtIndex(p.numJobs-schedulingStep);

	for(int i = p.numJobs - 1; i >= 0; i--) {
		int j = order[i];
		if(sts[j] == Project::UNSCHEDULED && p.allSuccsScheduled(j, sts))
			return j;
	}
	throw std::runtime_error("No eligible job found!");
}

int ActivityListPrioProvider::chooseEligibleWithLowestPriority(const Project &p, const std::vector<bool> &unscheduled, int schedulingStep) const {
	if (!robust) return getJobAtIndex(p.numJobs - schedulingStep);

	for(int i = p.numJobs - 1; i >= 0; i--) {
		int j = order[i];
		if(unscheduled[j] && p.allSuccsScheduled(j, unscheduled))
			return j;
	}
	throw std::runtime_error("No eligible job found!");
}

int ActivityListPrioProvider::chooseEligibleWithHighestPriority(const Project &p, const std::vector<int> &sts, int schedulingStep) const {
	if (!robust) return getJobAtIndex(schedulingStep);

	for(int j : order) {
		if (sts[j] == Project::UNSCHEDULED && p.allPredsScheduled(j, sts))
			return j;
	}
	throw std::runtime_error("No eligible job found!");
}

int ActivityListPrioProvider::chooseEligibleWithHighestPriority(const Project &p, const std::vector<bool> &unscheduled, int schedulingStep) const {
	if (!robust) return getJobAtIndex(schedulingStep);

	for (int j : order) {
		if (unscheduled[j] && p.allPredsScheduled(j, unscheduled))
			return j;
	}
	throw std::runtime_error("No eligible job found!");
}

int ActivityListPrioProvider::chooseEligibleWithHighestPrioAndResFeasInDt(const Project &p, int dt, const std::vector<int> &sts, const std::vector<int> &resRem) const {
	for(int j : order) {
		if(sts[j] == Project::UNSCHEDULED && p.allPredsScheduled(j, sts) && p.computeLastPredFinishingTimeForSts(sts, j) <= dt  && p.enoughCapacityForJobInFirstPeriod(j, resRem)) {
			return j;
		}
	}
	return -1;
}

int ActivityListPrioProvider::chooseEligibleWithHighestPrioAndResFeasRuntime(const Project &p, int dt, const std::vector<int> &sts, const Matrix<int> &resRem) const {
	for(int j : order) {
		if(sts[j] == Project::UNSCHEDULED && p.allPredsScheduled(j, sts) && p.computeLastPredFinishingTimeForSts(sts, j) <= dt && p.enoughCapacityForJob(j, dt, resRem)) {
			return j;
		}
	}
	return -1;
}

int ActivityListPrioProvider::getJobAtIndex(int k) const {
	return order[k];
}

RandomKeyPrioProvider::RandomKeyPrioProvider(const std::vector<float> &_priorities) : priorities(_priorities) {}

int RandomKeyPrioProvider::chooseEligibleWithLowestPriority(const Project &p, const std::vector<int> &sts, int schedulingStep) const {
	return 0;
}

int RandomKeyPrioProvider::chooseEligibleWithLowestPriority(const Project &p, const std::vector<bool> &unscheduled, int schedulingStep) const {
	return 0;
}

int RandomKeyPrioProvider::chooseEligibleWithHighestPriority(const Project &p, const std::vector<int> &sts, int schedulingStep) const {
	// higher is better
	int maxPrioJob = 0;
	float maxPrioVal = -1.0f;
	for(int i = 0; i < p.numJobs; i++) {
		if(sts[i] == Project::UNSCHEDULED && p.allPredsScheduled(i, sts) && priorities[i] > maxPrioVal) {
			maxPrioVal = priorities[i];
			maxPrioJob = i;
		}
	}
	return maxPrioJob;
}

int RandomKeyPrioProvider::chooseEligibleWithHighestPriority(const Project &p, const std::vector<bool> &unscheduled, int schedulingStep) const {
	return 0;
}

int RandomKeyPrioProvider::chooseEligibleWithHighestPrioAndResFeasInDt(const Project &p, int dt, const std::vector<int> &sts,
																	   const std::vector<int> &resRem) const {
	return 0;
}

int RandomKeyPrioProvider::chooseEligibleWithHighestPrioAndResFeasRuntime(const Project &p, int dt, const std::vector<int> &sts,
																		  const Matrix<int> &resRem) const {
	return 0;
}

int RandomKeyPrioProvider::getJobAtIndex(int k) const {
	return 0;
}