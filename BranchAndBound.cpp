#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p) : p(_p), lb(0.0f) {}

vector<int> BranchAndBound::solve() {
	vector<int> sts(p.numJobs, UNSCHEDULED);
	sts[0] = 0;
	branch(sts);
	return candidate;
}

bool BranchAndBound::isEligible(vector<int>& sts, int j) {
	for (int i = 0; i < p.numJobs; i++)
		if(p.adjMx(i,j) && sts[i] == UNSCHEDULED)
			return false;
	return true;
}

template <class Func>
bool BranchAndBound::resourceFeasibleCore(vector<int>& sts, int j, int stj, Func capacityLimit) {
	for (int r = 0; r < p.numRes; r++) {
		for (int tau = stj + 1; tau <= stj + p.durations[j]; tau++) {
			int cdemand = 0;
			for (int i = 0; i < p.numJobs; i++)
				if (sts[i] != UNSCHEDULED && sts[i] < tau && tau <= sts[i] + p.durations[i])
					cdemand += p.demands(i, r);

			if (cdemand + p.demands(j, r) > capacityLimit(r))
				return false;
		}
	}
	return false;
}

bool BranchAndBound::resourceFeasibleWithOvertime(vector<int>& sts, int j, int stj) {
	return resourceFeasibleCore(sts, j, stj, [&](int r) { return p.capacities[r] + p.zmax[r]; });
}

bool BranchAndBound::resourceFeasibleWithoutOvertime(vector<int>& sts, int j, int stj) {
	return resourceFeasibleCore(sts, j, stj, [&](int r) { return p.capacities[r]; });
}

float BranchAndBound::upperBoundForPartial(vector<int>& sts) {
	return numeric_limits<float>::max();
}

void BranchAndBound::branch(vector<int> sts) {
	for (int j = 0; j < p.numJobs; j++) {
		if (isEligible(sts, j)) {
			if (j == p.numJobs - 1) {
				sts[j] = sts[j - 1];
				candidate = sts;
				lb = Utils::max(lb, p.calcProfit(sts));
				return;
			}

			int t = 0;
			for (int i = 0; i < p.numJobs; i++) {
				if (p.adjMx(i, j))
					t = Utils::max(t, sts[i] + p.durations[i]);
			}

			for (;true;t++) {
				if (resourceFeasibleWithOvertime(sts, j, t)) {
					sts[j] = t;

					if (upperBoundForPartial(sts) > lb)
						branch(sts);
				}

				if (resourceFeasibleWithoutOvertime(sts, j, t))
					break;
			}
		}
	}
}