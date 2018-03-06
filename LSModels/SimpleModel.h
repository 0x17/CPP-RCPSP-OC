#pragma once

#include <vector>

class Project;

namespace LSSolver {
	std::vector<int> solveRCPSP(const Project &p);
}