#pragma once

#include <vector>
#include <string>

#include "Matrix.h"
#include "Stopwatch.h"
#include "Utils.h"

class ProjectWithOvertime;
class Stopwatch;

class BranchAndBound {
public:
	explicit BranchAndBound(ProjectWithOvertime& _p, double _timeLimit = 60.0, int _iterLimit = -1, bool _writeGraph = false);
    ~BranchAndBound();
	vector<int> solve(bool seedWithGA = false, bool traceobj = false, string outPath = "");

    static void solvePath(const string path);

	static string getTraceFilename(const string& outPath, const string& instanceName);

private:
	Stopwatch sw;
	ProjectWithOvertime &p;
	float lb;
	vector<int> candidate;
	int nodeCtr, boundCtr;
	string dotGraph, leafsStr;
	bool writeGraph;
    //TimePoint lupdate;
	double timeLimit;
	int iterLimit;
	std::unique_ptr<Utils::Tracer> tr;
	
	bool isEligible(vector<int> &sts, int j) const;
	std::pair<bool,bool> resourceFeasibilityCheck(vector<int> &sts, int j, int stj) const;
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
