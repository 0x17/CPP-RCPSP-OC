//
// Created by André Schnabel on 24.10.15.
//

#ifndef CPP_RCPSP_OC_LSSOLVER_H
#define CPP_RCPSP_OC_LSSOLVER_H

#include "Project.h"
#include "ProjectWithOvertime.h"

namespace LSSolver {
	vector<int> solve(ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
    vector<int> solveNative(ProjectWithOvertime &p);
	vector<int> solveListVarBinVar(int seed, ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
	vector<int> solveListVarFloatVar(int seed, ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
	vector<int> solveListVarIntVar(int seed, ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
	void writeLSPModelParamFile(ProjectWithOvertime &p, string outFilename);
};

#endif //CPP_RCPSP_OC_LSSOLVER_H
