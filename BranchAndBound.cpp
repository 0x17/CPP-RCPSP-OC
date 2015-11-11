#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include <algorithm>
#include <iostream>

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p) : p(_p), lb(0.0f), nodeCtr(0), boundCtr(0) {}

vector<int> BranchAndBound::solve() {
	nodeCtr = 0;
	boundCtr = 0;

	vector<int> sts(p.numJobs, p.UNSCHEDULED);
	sts[0] = 0;

	branch(sts);

	cout << "Number of nodes visited: " << nodeCtr << endl;
	cout << "Number of boundings: " << boundCtr << endl;

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

			cdemand += p.demands(j, r);

			if(cdemand > p.capacities[r]  + p.zmax[r])
				return make_pair(false, false);

			if(feasWoutOC && cdemand > p.capacities[r])
				feasWoutOC = false;
		}
	}
	return make_pair(true, feasWoutOC);
}

float BranchAndBound::upperBoundForPartial(vector<int>& sts) {
	return p.revenue[p.makespan(p.earliestStartingTimesForPartial(sts))] - p.totalCosts(p.serialSGSForPartial(sts, p.topOrder).second);
}

void BranchAndBound::branch(vector<int> sts) {
	nodeCtr++;

	for(int j = 0; j < p.numJobs; j++) {
		if(isEligible(sts, j)) {
			if(j == p.numJobs - 1) {
				sts[p.lastJob] = 0;
				p.eachJobConst([&](int i) {
					if(i < p.lastJob)
						sts[p.lastJob] = Utils::max(sts[i] + p.durations[i], sts[p.lastJob]);
				});
				float profit = p.calcProfit(sts);
				if(profit > lb) {
					candidate = sts;
					lb = profit;
					cout << "Updated lower bound = " << lb << endl;
				}
				return;
			}

			int t = 0;
			for(int i = 0; i < p.numJobs; i++) {
				if (p.adjMx(i, j))
					t = Utils::max(t, sts[i] + p.durations[i]);
			}

			for(;true;t++) {
				pair<bool, bool> feas = resourceFeasibilityCheck(sts, j, t);

				if(feas.first) {
					if(upperBoundForPartial(sts) > lb) {
						sts[j] = t;
						branch(sts);
						sts[j] = p.UNSCHEDULED;
					}
					else
						boundCtr++;

				}

				if(feas.second)
					break;
			}
		}
	}
}