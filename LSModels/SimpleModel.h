#pragma once

#ifndef DISABLE_LOCALSOLVER

#include <vector>
#include "../ProjectWithOvertime.h"

class Project;

namespace LSSolver {
	std::vector<int> solveRCPSP(const Project &p);
	std::vector<int> solveRCPSPROC(const ProjectWithOvertime &p);
}

#endif