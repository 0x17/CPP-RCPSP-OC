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
    TimePoint lupdate;
	
	bool isEligible(vector<int> &sts, int j);
	pair<bool,bool> resourceFeasibilityCheck(vector<int> &sts, int j, int stj);
	void branch(vector<int> sts, int job, int stj);
    void foundLeaf(vector<int> &sts);
    
    float upperBoundForPartial(const vector<int> &sts) const;
    float upperBoundForPartial2(const vector<int> &sts) const;
    float upperBoundForPartialSimple(const vector<int> &sts) const;
    struct AreaData;
    AreaData computeAreas(const vector<int> &sts, const Matrix<int> &resRem, int tmin, int tmax) const;
    float costsLbForMakespan(int msMin, const vector<int> &missingDemand, const vector<int> &freeArea, vector<int> &overtime, int ms) const;

	void addArrowToGraph(int nodeA, int nodeB);
	void addLeafToGraph(int node, const vector<int> &sts);
	void addNodeLabelToGraph(int node, const vector<int> &sts);
	void drawGraph();
	void graphPreamble();
};

#endif