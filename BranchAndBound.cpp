#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include <algorithm>

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

pair<bool,bool> BranchAndBound::resourceFeasibilityCheck(vector<int>& sts, int j, int stj) {
	bool feasWoutOC = true;

	for(int r = 0; r < p.numRes; r++) {
		for(int tau = stj + 1; tau <= stj + p.durations[j]; tau++) {
			int cdemand = 0;
			for(int i = 0; i < p.numJobs; i++)
				if (sts[i] != UNSCHEDULED && sts[i] < tau && tau <= sts[i] + p.durations[i])
					cdemand += p.demands(i, r);

			if(cdemand + p.demands(j, r) > p.capacities[r]  + p.zmax[r])
				return make_pair(false, false);

			if(feasWoutOC && cdemand + p.demands(j,r) > p.capacities[r])
				feasWoutOC = false;
		}
	}
	return make_pair(true, feasWoutOC);
}

float BranchAndBound::upperBoundForPartial(vector<int>& sts) {
	return numeric_limits<float>::max();
}

// DEBUG ME!
void BranchAndBound::branch(vector<int> sts) {
	for (int j = 0; j < p.numJobs; j++) {
		if (isEligible(sts, j)) {
			if (j == p.numJobs - 1) {
				sts[j] = sts[j - 1];
				candidate = sts;
				lb = max(lb, p.calcProfit(sts));
				return;
			}

			int t = 0;
			for (int i = 0; i < p.numJobs; i++) {
				if (p.adjMx(i, j))
					t = Utils::max(t, sts[i] + p.durations[i]);
			}

			for (;true;t++) {
				pair<bool, bool> feas = resourceFeasibilityCheck(sts, j, t);

				if (feas.first) {
					sts[j] = t;

					if (upperBoundForPartial(sts) > lb)
						branch(sts);

					sts[j] = UNSCHEDULED;
				}

				if (feas.second)
					break;
			}
		}
	}
}