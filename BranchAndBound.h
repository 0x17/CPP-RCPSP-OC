#ifndef __BRANCH_AND_BOUND_H__
#define __BRANCH_AND_BOUND_H__

#include <vector>
#include "Stopwatch.h"
#include "matrix.h"

using namespace std;

class ProjectWithOvertime;

class BranchAndBound {
public:
	explicit BranchAndBound(ProjectWithOvertime& _p);
	vector<int> solve();

private:
	Stopwatch sw;
	ProjectWithOvertime &p;
	float lb;
	vector<int> candidate;
	int nodeCtr, boundCtr;

	bool isEligible(vector<int> &sts, int j);
	pair<bool,bool> resourceFeasibilityCheck(vector<int> &sts, int j, int stj);
	float upperBoundForPartial(vector<int> &sts);
    float upperBoundForPartialSimple(vector<int> &sts);
	void branch(vector<int> sts);
    void foundLeaf(vector<int> &sts);
    pair<vector<int>, vector<int>> computeAreas(const vector<int> &sts, const Matrix<int> &resRem, int msMin) const;
    float costsLbForMakespan(int msMin, const vector<int> &missingDemand, const vector<int> &freeArea, int ms) const;
};

#endif