﻿#ifndef __BRANCH_AND_BOUND_H__
#define __BRANCH_AND_BOUND_H__

#include <vector>
using namespace std;

class ProjectWithOvertime;

class BranchAndBound {
public:
	explicit BranchAndBound(ProjectWithOvertime& _p);
	vector<int> solve();

private:
	ProjectWithOvertime &p;
	float lb;
	vector<int> candidate;

	bool isEligible(vector<int> &sts, int j);
	pair<bool,bool> resourceFeasibilityCheck(vector<int> &sts, int j, int stj);
	float upperBoundForPartial(vector<int> &sts);
	void branch(vector<int> sts);
};

#endif