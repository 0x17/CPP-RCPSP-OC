#pragma once

#ifndef DISABLE_GUROBI

#include <vector>
#include <gurobi_c++.h>

#include "Matrix.h"
#include "Utils.h"
#include "Logger.h"
#include "BasicSolverParameters.h"

class ProjectWithOvertime;

class GurobiSolver {
public:	
	struct Options : BasicSolverParameters {
		bool useSeedSol;
		double gap;
		int displayInterval;
		Options();
	};

	struct Result {
		std::vector<int> sts;
		bool optimal;
		Result(std::vector<int> &_sts, bool _optimal);
	};

	GurobiSolver(ProjectWithOvertime& _p, Options _opts);

	void restrictJobToTimeWindow(int j, int eft, int lft);

	void relaxJob(int j);
	void relaxAllJobs();

	Result solve();

	static std::string traceFilenameForInstance(const std::string& outPath, const std::string& instanceName);

private:
	class CustomCallback : public GRBCallback {
	public:
		CustomCallback(const std::string &outPath, const std::string& instanceName);
		void manualCallback(float bks);

	private:
		void callback() override;
		Utils::Tracer tr;
		Stopwatch sw;
	};

	ProjectWithOvertime &p;
	Options opts;
	GRBEnv env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	CustomCallback cback;

	static GRBEnv setupOptions(Options opts);

	void setupObjectiveFunction();
	void setupConstraints();
	void setupFeasibleMipStart();

	std::vector<int> parseSchedule() const;
};

#endif