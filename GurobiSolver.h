#pragma once

#define USE_GUROBI 1

#include <vector>
#include <gurobi_c++.h>
#include "Matrix.h"

class ProjectWithOvertime;

class GurobiSolver {
public:
	GurobiSolver(ProjectWithOvertime& _p);
	vector<int> solve();

private:
	ProjectWithOvertime &p;
	GRBEnv env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	
	void setupObjectiveFunction();
	void setupConstraints();

	vector<int> parseSchedule() const;
};
