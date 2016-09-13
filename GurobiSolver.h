#pragma once

#define USE_GUROBI 1

#include <vector>
#include <gurobi_c++.h>
#include "Matrix.h"
#include "Utils.h"

class ProjectWithOvertime;

class GurobiSolver {
public:	
	struct Options {
		string outPath;
		bool useSeedSol;
		double timeLimit, gap;
		int displayInterval;
		Options();
	};

	GurobiSolver(ProjectWithOvertime& _p, Options _opts);

	void restrictJobToTimeWindow(int j, int eft, int lft);
	void relaxJob(int j);
	void relaxAllJobs();

	vector<int> solve();

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
	GRBEnv env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	CustomCallback cback;
	Options opts;

	void setupOptions();
	void setupObjectiveFunction();
	void setupConstraints();
	void setupFeasibleMipStart();

	vector<int> parseSchedule() const;
};
