#ifndef __BRANCH_AND_BOUND_H__
#define __BRANCH_AND_BOUND_H__

#include <vector>
#include "matrix.h"
#include "Stopwatch.h"
#include <string>

using namespace std;

class ProjectWithOvertime;
class Stopwatch;

class BranchAndBound {
public:
	explicit BranchAndBound(ProjectWithOvertime& _p, bool _writeGraph = false);
	vector<int> solve(bool seedWithGA = false);

private:
	Stopwatch sw;
	ProjectWithOvertime &p;
	float lb;
	vector<int> candidate;
	int nodeCtr, boundCtr;
	string dotGraph, leafsStr;
	bool writeGraph;
	
	bool isEligible(vector<int> &sts, int j);
	pair<bool,bool> resourceFeasibilityCheck(vector<int> &sts, int j, int stj);
	float upperBoundForPartial(vector<int> &sts);
    float upperBoundForPartialSimple(vector<int> &sts);
	void branch(vector<int> sts, int job, int stj);
    void foundLeaf(vector<int> &sts);
    pair<vector<int>, vector<int>> computeAreas(const vector<int> &sts, const Matrix<int> &resRem, int msMin) const;
    float costsLbForMakespan(int msMin, const vector<int> &missingDemand, const vector<int> &freeArea, int ms) const;

	void addArrowToGraph(int nodeA, int nodeB);
	void addLeafToGraph(int node, const vector<int> &sts);
	void addNodeLabelToGraph(int node, const vector<int> &sts);
	void drawGraph();
	void graphPreamble();
};

#endif