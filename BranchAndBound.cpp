#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <map>
#include <string>

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p, bool _writeGraph) : p(_p), lb(numeric_limits<float>::lowest()), nodeCtr(0), boundCtr(0), writeGraph(_writeGraph) {}

vector<int> BranchAndBound::solve(bool seedWithGA) {
    lupdate = chrono::system_clock::now();
    sw.start();
    
	if (seedWithGA) {
		FixedCapacityGA ga(p);
		auto res = ga.solve();
		candidate = res.first;
		lb = res.second;
		cout << "Lower bound seeded by genetic algorithm = " << lb << endl;
	}

	nodeCtr = 0;
	boundCtr = 0;

	graphPreamble();

	vector<int> sts(p.numJobs, Project::UNSCHEDULED);
	branch(sts, 0, 0);
    
    double solvetime = sw.look();

	cout << "Number of nodes visited: " << nodeCtr << endl;
	cout << "Number of boundings: " << boundCtr << endl;
	cout << "Total solvetime: " << solvetime << endl;

	drawGraph();

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
	Matrix<int> resRemCp = resRem;

	int msMin = p.makespan(p.earliestStartingTimesForPartial(sts));
	int msMax = p.makespan(p.serialSGSForPartial(sts, p.topOrder, resRem).first);

    auto areas = computeAreas(sts, resRemCp, msMin);
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
			freeArea[r] += Utils::max(0, resRem(r, t));
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
	if(chrono::duration<double, std::milli>(chrono::system_clock::now() - lupdate).count() > 1000.0) {
		cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << endl;
        lupdate = chrono::system_clock::now();
	}

	sts[job] = stj;

 	nodeCtr++;
	int nodeIx = nodeCtr;
	addNodeLabelToGraph(nodeIx, sts);

    int maxSt = p.latestStartingTimeInPartial(sts);

	for(int j = 0; j < p.numJobs; j++) {
		if(isEligible(sts, j)) {
			if(j == p.numJobs - 1) {
                foundLeaf(sts);
				addLeafToGraph(nodeIx, sts);
                return;
			}
			
			list<pair<float, int>> ubToT;

			for(int t = p.computeLastPredFinishingTimeForSts(sts, j); true; t++) {
				pair<bool, bool> feas = resourceFeasibilityCheck(sts, j, t);

				if(feas.first) {
					// fathom redundant schedules
					if (t < maxSt) {
						boundCtr++;
						break;
					}
					
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
			for(auto p : ubToT) {
				addArrowToGraph(nodeIx, nodeCtr + 1);
				branch(sts, j, p.second);
			}
		}
	}
}

void BranchAndBound::addArrowToGraph(int nodeA, int nodeB) {
    if (!writeGraph) return;
    dotGraph += to_string(nodeA) + "->" + to_string(nodeB) + "\n";
}

void BranchAndBound::addLeafToGraph(int node, const vector<int> &sts) {
    if (!writeGraph) return;
    string stsStr;
    for(int i = 0; i < sts.size(); i++)
        stsStr += to_string(sts[i]) + (i + 1 < sts.size() ? "," : "");
    leafsStr += (to_string(node) + "[label=\"" + stsStr + "\"]\n");
}

void BranchAndBound::addNodeLabelToGraph(int node, const vector<int>& sts) {
    if (!writeGraph) return;
    string stsStr;
    for (int i = 0; i < sts.size(); i++)
        stsStr += ((sts[i] == Project::UNSCHEDULED) ? "_" : to_string(sts[i])) + (i + 1 < sts.size() ? "," : "") + ((i+1) % 4 == 0 ? "\n" : "");
    leafsStr += (to_string(node) + "[label=\"#" + to_string(node) + "\n" + stsStr + "\"]\n");
}

void BranchAndBound::drawGraph() {
    if(!writeGraph) return;
    
    dotGraph += leafsStr;
    dotGraph += "\n}";
    
    const string dotFilename = "bbgraph.dot";
    Utils::spit(dotGraph, dotFilename);
    system(("dot -Tpdf " + dotFilename + " -o" + dotFilename + ".pdf").c_str());
}

void BranchAndBound::graphPreamble() {
    if(!writeGraph) return;
    dotGraph = "digraph precedence{\n";
}