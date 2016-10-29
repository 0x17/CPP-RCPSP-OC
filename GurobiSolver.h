#pragma once

#ifndef DISABLE_GUROBI

#include <vector>
#include "Matrix.h"
#include "Utils.h"
#include <gurobi_c++.h>

class ProjectWithOvertime;

class GurobiSolver {
public:	
	struct Options : Utils::BasicSolverParameters {
		bool useSeedSol;
		double gap;
		int displayInterval;
		Options();
	};

	struct Result {
		vector<int> sts;
		bool optimal;
		Result(vector<int> sts, bool optimal);
	};

	GurobiSolver(ProjectWithOvertime& _p, Options _opts);

	void restrictJobToTimeWindow(int j, int eft, int lft);
	void relaxJob(int j);
	void relaxAllJobs();

	Result solve();

private:
	class CustomCallback : public GRBCallback {
	public:
		CustomCallback(string outPath, string instanceName);
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

	vector<int> parseSchedule() const;
};

#endif