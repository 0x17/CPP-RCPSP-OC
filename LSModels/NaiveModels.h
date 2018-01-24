#pragma once

#include <localsolver.h>

#include "../Project.h"
#include "../ProjectWithOvertime.h"

namespace LSSolver {
	std::vector<int> solve(ProjectWithOvertime &p, double timeLimit = 60.0, int iterLimit = -1, bool traceobj = false, std::string outPath = "");
	std::vector<int> solveNative(ProjectWithOvertime &p);
	std::vector<int> solvePartitionListModel(ProjectWithOvertime &p);
	void writeLSPModelParamFile(ProjectWithOvertime &p, std::string outFilename);
};