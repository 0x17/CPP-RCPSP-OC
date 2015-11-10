#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include <algorithm>

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p) : p(_p), lb(0.0f), Tmin(p.makespan(p.ests)) {}

vector<int> BranchAndBound::solve() {
	vector<int> sts(p.numJobs, p.UNSCHEDULED);
	sts[0] = 0;
	branch(sts);
	return candidate;
}

bool BranchAndBound::isEligible(vector<int>& sts, int j) {
    if(sts[j] != p.UNSCHEDULED)
        return false;

	for (int i = 0; i < p.numJobs; i++)
		if(p.adjMx(i,j) && sts[i] == p.UNSCHEDULED)
			return false;
	return true;
}

pair<bool,bool> BranchAndBound::resourceFeasibilityCheck(vector<int>& sts, int j, int stj) {
	bool feasWoutOC = true;

	for(int r = 0; r < p.numRes; r++) {
		for(int tau = stj + 1; tau <= stj + p.durations[j]; tau++) {
			int cdemand = 0;
			for(int i = 0; i < p.numJobs; i++)
				if (sts[i] != p.UNSCHEDULED && sts[i] < tau && tau <= sts[i] + p.durations[i])
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
    int Tmax = p.makespan(p.serialSGSForPartial(sts, p.topOrder));
    float maxProfit = 0.0f;
    float minCosts = p.totalCostsForPartial(sts);

    for(int deadline = Tmin; deadline <= Tmax; deadline++) {
        // TODO BETTER MINCOST
        minCosts = 0.0f;
        maxProfit = max(p.revenue[deadline] - minCosts, maxProfit);
    }

	return maxProfit;
}

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

					sts[j] = p.UNSCHEDULED;
				}

				if (feas.second)
					break;
			}
		}
	}
}