//
// Created by André Schnabel on 10.10.17.
//

#pragma once

#include <string>

struct BasicSolverParameters {
	double timeLimit;
	int iterLimit;
	bool traceobj;
	std::string outPath;
	int threadCount;

	BasicSolverParameters(double time_limit, int iter_limit, bool traceobj, const std::string& out_path, int thread_count);
};

struct SolverParams : BasicSolverParameters {
	int seed, verbosityLevel, solverIx;
	explicit SolverParams(double _tlimit = -1.0, int _ilimit = -1);
};

class ISolver {
public:
	virtual ~ISolver() = default;
	virtual std::vector<int> solve(SolverParams params) = 0;
};