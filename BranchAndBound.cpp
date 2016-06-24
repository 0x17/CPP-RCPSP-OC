#include <algorithm>
#include <numeric>
#include <map>
#include <cmath>
#include <fstream>

#include "BranchAndBound.h"
#include "Utils.h"
#include "ProjectWithOvertime.h"
#include "GeneticAlgorithms/OvertimeBound.h"

BranchAndBound::BranchAndBound(ProjectWithOvertime& _p, double _timeLimit, int _iterLimit, bool _writeGraph)
	: p(_p), lb(numeric_limits<float>::lowest()), nodeCtr(0), boundCtr(0), writeGraph(_writeGraph), timeLimit(_timeLimit), iterLimit(_iterLimit), traceobj(false), tr(nullptr) {}

BranchAndBound::~BranchAndBound() {
    if(tr != nullptr) delete tr;
}

vector<int> BranchAndBound::solve(bool seedWithGA, bool traceobj) {
    this->traceobj = traceobj;
    if(traceobj && tr == nullptr) {
        tr = new Utils::Tracer("BranchAndBoundTrace_" + p.instanceName);
    }

    lupdate = chrono::system_clock::now();
    sw.start();
    
	if (seedWithGA) {
		FixedCapacityGA ga(p);
		auto res = ga.solve();
		candidate = res.first;
		lb = res.second;
		cout << endl << "Lower bound seeded by genetic algorithm = " << lb << endl;
	} else {
        candidate = p.serialSGS(p.topOrder);
        lb = p.calcProfit(candidate);
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

bool BranchAndBound::isEligible(vector<int>& sts, int j) const {
    if(sts[j] != Project::UNSCHEDULED)
        return false;

	for (int i = 0; i < p.numJobs; i++)
		if(p.adjMx(i,j) && sts[i] == Project::UNSCHEDULED)
			return false;

	return true;
}

pair<bool,bool> BranchAndBound::resourceFeasibilityCheck(vector<int>& sts, int j, int stj) const {
	bool feasWoutOC = true;
	for(int r = 0; r < p.numRes; r++) {
		for(int tau = stj + 1; tau <= stj + p.durations[j]; tau++) {
			int cdemand = 0;
			for(int i = 0; i < p.numJobs; i++)
				if (sts[i] != Project::UNSCHEDULED && sts[i] < tau && tau <= sts[i] + p.durations[i])
					cdemand += p.demands(i, r);

			cdemand += p.demands(j, r);

			if(cdemand > p.capacities[r] + p.zmax[r])
				return make_pair(false, false);

			if(feasWoutOC && cdemand > p.capacities[r])
				feasWoutOC = false;
		}
	}
	return make_pair(true, feasWoutOC);
}

struct BranchAndBound::AreaData {
    vector<int> missingDemand, freeArea, overtime;
    AreaData(Project &p) : missingDemand(p.numRes), freeArea(p.numRes), overtime(p.numRes) {}
};

float BranchAndBound::upperBoundForPartial(const vector<int>& sts) const {
	Matrix<int> resRem = p.resRemForPartial(sts);
	Matrix<int> resRemCp = resRem;

	int msMin = p.makespan(p.earliestStartingTimesForPartial(sts));
	int msMax = p.makespan(p.serialSGSForPartial(sts, p.topOrder, resRem).first);

    AreaData data = computeAreas(sts, resRemCp, 0, msMin);

    return Utils::maxInRangeIncl(msMin, msMax, [&](int ms) { return p.revenue[ms] - costsLbForMakespan(msMin, data.missingDemand, data.freeArea, data.overtime, ms); });
}

float BranchAndBound::costsLbForMakespan(int msMin, const vector<int> &missingDemand, const vector<int> &freeArea, vector<int> &overtime, int ms) const {
    float costs = 0.0f;
    for (int r = 0; r < p.numRes; r++) {
        costs += p.kappa[r] * Utils::max(0, overtime[r] + missingDemand[r] - (freeArea[r] + (ms - msMin) * p.capacities[r]));
    }
    return costs;
}

BranchAndBound::AreaData BranchAndBound::computeAreas(const vector<int> &sts, const Matrix<int> &resRem, int tmin, int tmax) const {
    AreaData data(p);
    for (int r = 0; r < p.numRes; r++) {
		data.missingDemand[r] = 0;
		for (int j = 0; j < p.numJobs; j++)
			if (sts[j] == Project::UNSCHEDULED)
                data.missingDemand[r] += p.demands(j, r) * p.durations[j];

        data.freeArea[r] = 0;
        data.overtime[r] = 0;
        for (int t = tmin; t <= tmax; t++) {
            data.freeArea[r] += Utils::max(0, resRem(r, t));
            data.overtime[r] += Utils::max(0, -resRem(r, t));
        }
	}
    return data;
}

float BranchAndBound::upperBoundForPartialSimple(const vector<int> &sts) const {
    return p.revenue[p.makespan(p.earliestStartingTimesForPartial(sts))] - p.totalCostsForPartial(sts);
}

float BranchAndBound::upperBoundForPartial2(const vector<int> &sts) const {
    Matrix<int> resRem = p.resRemForPartial(sts);
    float fixedCosts = p.totalCostsForPartial(sts);

	vector<int> essFeas = p.earliestStartingTimesForPartialRespectZmax(sts, resRem);

	int minStUnscheduled = numeric_limits<int>::max();
	for (int j = 0; j < p.numJobs; j++) {
		if (sts[j] == Project::UNSCHEDULED && essFeas[j] < minStUnscheduled)
			minStUnscheduled = essFeas[j];
	}

    int essFeasMakespan = p.makespan(essFeas);

    AreaData data = computeAreas(sts, resRem, minStUnscheduled, essFeasMakespan);

    float bestProfit = numeric_limits<float>::lowest();
    int delayPeriods = 0;

    while(true) {
        float additionalCostsLb = 0.0f;
        for (int r = 0; r < p.numRes; r++) {
            additionalCostsLb += static_cast<float>(fmax(0, data.missingDemand[r] - (data.freeArea[r] + p.capacities[r] * delayPeriods))) * p.kappa[r];
        }

        float profit = p.revenue[essFeasMakespan + delayPeriods] - (fixedCosts + additionalCostsLb);
        bestProfit = fmax(profit, bestProfit);

        if(fabs(additionalCostsLb - 0.0f) < 0.0000001f) {
            break;
        }

        delayPeriods++;
    }

    return bestProfit;
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
		if(tr != nullptr) tr->trace(sw.look(), lb);
    }
}

void BranchAndBound::branch(vector<int> sts, int job, int stj) {
	if (timeLimit != -1.0 && sw.look() >= timeLimit * 1000.0
		|| (iterLimit != -1 && nodeCtr >= iterLimit))
		return;

	if(chrono::duration<double, std::milli>(chrono::system_clock::now() - lupdate).count() > MSECS_BETWEEN_TRACES) {
		cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
        lupdate = chrono::system_clock::now();
        if(traceobj) tr->trace(sw.look(), lb);
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
					/*if (t < maxSt) {
						boundCtr++;
						continue;
					}*/
					
					sts[j] = t;
                    float ub = upperBoundForPartialSimple(sts);
					sts[j] = Project::UNSCHEDULED;

					// fathom proven suboptimal schedules
					if(ub > lb) ubToT.push_back(make_pair(-ub, t));
					else boundCtr++;
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

void BranchAndBound::solvePath(const string path) {
    auto instanceFilenames = Utils::filenamesInDirWithExt(path, ".sm");
    ofstream outFile("branchandboundresults.csv");
    if(!outFile.is_open()) return;
    for(auto instanceFn : instanceFilenames) {
        ProjectWithOvertime p(instanceFn);
        BranchAndBound bandb(p);
        auto sts = bandb.solve(true);
        outFile << instanceFn << ";" << to_string(p.calcProfit(sts)) << endl;
    }
    outFile.close();
}

