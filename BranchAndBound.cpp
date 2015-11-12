﻿#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <map>

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p) : p(_p), lb(numeric_limits<float>::lowest()), nodeCtr(0), boundCtr(0) {}

vector<int> BranchAndBound::solve() {
	sw.start();

	nodeCtr = 0;
	boundCtr = 0;

	vector<int> sts(p.numJobs, Project::UNSCHEDULED);
	branch(sts, 0, 0);

	cout << "Number of nodes visited: " << nodeCtr << endl;
	cout << "Number of boundings: " << boundCtr << endl;
	cout << "Total solvetime: " << sw.look() << endl;

	return candidate;
}

bool BranchAndBound::isEligible(vector<int>& sts, int j) {
    if(sts[j] != Project::UNSCHEDULED)
        return false;

	for (int i = 0; i < p.numJobs; i++)
		if(p.adjMx(i,j) && sts[i] == Project::UNSCHEDULED)
			return false;

	return true;
}

pair<bool,bool> BranchAndBound::resourceFeasibilityCheck(vector<int>& sts, int j, int stj) {
	bool feasWoutOC = true;
	for(int r = 0; r < p.numRes; r++) {
		for(int tau = stj + 1; tau <= stj + p.durations[j]; tau++) {
			int cdemand = 0;
			for(int i = 0; i < p.numJobs; i++)
				if (sts[i] != Project::UNSCHEDULED && sts[i] < tau && tau <= sts[i] + p.durations[i])
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

    auto areas = computeAreas(sts, resRem, msMin);
    vector<int> &missingDemand = areas.first;
    vector<int> &freeArea = areas.second;

    return Utils::maxInRangeIncl(msMin, msMax, [&](int ms) { return p.revenue[ms] - costsLbForMakespan(msMin, missingDemand, freeArea, ms); });
}

float BranchAndBound::costsLbForMakespan(int msMin, const vector<int> &missingDemand, const vector<int> &freeArea, int ms) const {
    float costs = 0.0f;
    for (int r = 0; r < p.numRes; r++) {
        costs += p.kappa[r] * Utils::max(0, missingDemand[r] - (freeArea[r] + (ms - msMin) * p.capacities[r]));
    }
    return costs;
}

pair<vector<int>, vector<int>> BranchAndBound::computeAreas(const vector<int> &sts, const Matrix<int> &resRem, int msMin) const {
    vector<int> missingDemand(p.numRes), freeArea(p.numRes);
    for (int r = 0; r < p.numRes; r++) {
		missingDemand[r] = 0;
		for (int j = 0; j < p.numJobs; j++)
			if (sts[j] == Project::UNSCHEDULED)
				missingDemand[r] += p.demands(j, r) * p.durations[j];

		freeArea[r] = 0;
		for (int t = 0; t <= msMin; t++)
			freeArea[r] += max(0, resRem(r, t));
	}
    return make_pair(missingDemand, freeArea);
}

float BranchAndBound::upperBoundForPartialSimple(vector<int> &sts) {
    return p.revenue[p.makespan(p.earliestStartingTimesForPartial(sts))] - p.totalCostsForPartial(sts);
}

void BranchAndBound::foundLeaf(vector<int> &sts) {
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
}

void BranchAndBound::branch(vector<int> sts, int job, int stj) {
	if(static_cast<int>(sw.look()) % 1000 == 0) {
		cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << endl;
	}

	sts[job] = stj;

 	nodeCtr++;

    int maxSt = p.latestStartingTimeInPartial(sts);

	for(int j = 0; j < p.numJobs; j++) {
		if(isEligible(sts, j)) {
			if(j == p.numJobs - 1) {
                foundLeaf(sts);
                return;
			}
						
			int tPredFeas = p.computeLastPredFinishingTimeForSts(sts, j);

			// fathom redundant schedules
			if(tPredFeas < maxSt) {
				boundCtr++;
				continue;
			}

			list<pair<float, int>> ubToT;

			for(int t = tPredFeas; true; t++) {
				pair<bool, bool> feas = resourceFeasibilityCheck(sts, j, t);

				if(feas.first) {
					//if (upperBoundForPartialSimple(sts) > lb) branch(sts);
					//else boundCtr++;
					
					sts[j] = t;
					float ub = upperBoundForPartialSimple(sts);
					sts[j] = Project::UNSCHEDULED;

					// fathom proven suboptimal schedules
					if(ub > lb) {
						ubToT.push_back(make_pair(-ub, t));
						boundCtr++;
					}
				}

				if(feas.second) break;
			}

			ubToT.sort();
			for(auto p : ubToT)
				branch(sts, j, p.second);
		}
	}
}
