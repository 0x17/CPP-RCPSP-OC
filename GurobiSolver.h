#pragma once

#define USE_GUROBI 1

#include <vector>

class ProjectWithOvertime;

namespace GurobiSolver {	
	std::vector<int> solve(ProjectWithOvertime& p);
}