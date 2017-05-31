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
	std::vector<int> solve(bool seedWithGA = false, bool traceobj = false, std::string outPath = "");

    static void solvePath(const std::string path);

	static std::string getTraceFilename(const std::string& outPath, const std::string& instanceName);

private:
	Stopwatch sw;
	ProjectWithOvertime &p;
	float lb;
	std::vector<int> candidate;
	int nodeCtr, boundCtr;
	std::string dotGraph, leafsStr;
	bool writeGraph;
    //TimePoint lupdate;
	double timeLimit;
	int iterLimit;
	std::unique_ptr<Utils::Tracer> tr;
	
	bool isEligible(std::vector<int> &sts, int j) const;
	std::pair<bool,bool> resourceFeasibilityCheck(std::vector<int> &sts, int j, int stj) const;
	void branch(std::vector<int> sts, int job, int stj);
    void foundLeaf(std::vector<int> &sts);
    
    float upperBoundForPartial(const std::vector<int> &sts) const;
    float upperBoundForPartial2(const std::vector<int> &sts) const;
    float upperBoundForPartialSimple(const std::vector<int> &sts) const;
    struct AreaData;
    AreaData computeAreas(const std::vector<int> &sts, const Matrix<int> &resRem, int tmin, int tmax) const;
    float costsLbForMakespan(int msMin, const std::vector<int> &missingDemand, const std::vector<int> &freeArea, std::vector<int> &overtime, int ms) const;

	void addArrowToGraph(int nodeA, int nodeB);
	void addLeafToGraph(int node, const std::vector<int> &sts);
	void addNodeLabelToGraph(int node, const std::vector<int> &sts);
	void drawGraph();
	void graphPreamble();
};
