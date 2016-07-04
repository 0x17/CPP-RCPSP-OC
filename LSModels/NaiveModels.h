#pragma once

#include <localsolver.h>

#include "../Project.h"
#include "../ProjectWithOvertime.h"

using namespace localsolver;

namespace LSSolver {
	vector<int> solve(ProjectWithOvertime &p, double timeLimit = 60.0, int iterLimit = -1, bool traceobj = false, string outPath = "");
	vector<int> solveNative(ProjectWithOvertime &p);
	void writeLSPModelParamFile(ProjectWithOvertime &p, string outFilename);
};