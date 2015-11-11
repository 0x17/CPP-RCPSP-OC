#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include <algorithm>
#include <iostream>
#include <numeric>

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p) : p(_p), lb(0.0f), nodeCtr(0), boundCtr(0) {}

vector<int> BranchAndBound::solve() {
	sw.start();

	nodeCtr = 0;
	boundCtr = 0;

	vector<int> sts(p.numJobs, p.UNSCHEDULED);
	sts[0] = 0;

	branch(sts);

	cout << "Number of nodes visited: " << nodeCtr << endl;
	cout << "Number of boundings: " << boundCtr << endl;
	cout << "Total solvetime: " << sw.look() << endl;

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
	Matrix<int> resRem = p.resRemForPartial(sts);

	int msMin = p.makespan(p.earliestStartingTimesForPartial(sts));
	int msMax = p.makespan(p.serialSGSForPartial(sts, p.topOrder, resRem).first);
	
	float bestProfit = numeric_limits<float>::lowest(), costs;

	vector<int> missingDemand(p.numRes), freeArea(p.numRes);
	for (int r = 0; r < p.numRes; r++) {
		missingDemand[r] = 0;
		for (int j = 0; j < p.numJobs; j++)
			if (sts[j] == p.UNSCHEDULED)
				missingDemand[r] += p.demands(j, r);

		freeArea[r] = 0;
		for (int t = 0; t <= msMin; t++)
			freeArea[r] += max(0, resRem(r, t));
	}	

	for(int ms = msMin; ms <= msMax; ms++) {
		costs = 0.0f;
		for (int r = 0; r < p.numRes; r++) {			
			costs += p.kappa[r] * Utils::max(0, missingDemand[r] - (freeArea[r] + (ms-msMin) * p.capacities[r]));
		}

		float profit = p.revenue[ms] - costs;
		if (profit > bestProfit)
			bestProfit = profit;		
	}

	return bestProfit;

	//return p.revenue[p.makespan(p.earliestStartingTimesForPartial(sts))] - p.totalCostsForPartial(sts);
}

void BranchAndBound::branch(vector<int> sts) {
	if(static_cast<int>(sw.look()) % 1000 == 0) {
		cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << endl;
	}

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