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
