//
// Created by André Schnabel on 10.10.17.
//

#include "BasicSolverParameters.h"

using namespace std;

BasicSolverParameters::BasicSolverParameters(double time_limit, int iter_limit, bool traceobj, const string& out_path, int thread_count): timeLimit(time_limit),
																																		  iterLimit(iter_limit),
																																		  traceobj(traceobj),
																																		  outPath(out_path),
																																		  threadCount(thread_count) {
}

SolverParams::SolverParams(double _tlimit, int _ilimit): BasicSolverParameters(_tlimit, _ilimit, false, "LocalSolverNative_", 1), seed(0), verbosityLevel(2), solverIx(0) {
}