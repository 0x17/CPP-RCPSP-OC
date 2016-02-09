#pragma once

#include <localsolver.h>

#include "../Project.h"
#include "../ProjectWithOvertime.h"

using namespace localsolver;

namespace LSSolver {
	vector<int> solve(ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
	vector<int> solveNative(ProjectWithOvertime &p);
	void writeLSPModelParamFile(ProjectWithOvertime &p, string outFilename);
};