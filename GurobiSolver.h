#pragma once

#define USE_GUROBI 1

#include <vector>
#include <gurobi_c++.h>
#include "Matrix.h"
#include "Utils.h"

class ProjectWithOvertime;

class GurobiSolver {
public:
	GurobiSolver(ProjectWithOvertime& _p);

	void restrictJobToTimeWindow(int j, int eft, int lft);
	void relaxJob(int j);

	vector<int> solve();

private:
	class CustomCallback : public GRBCallback {
	public:
		CustomCallback();
	private:
		void callback() override;
		Utils::Tracer tr;
	};

	ProjectWithOvertime &p;
	GRBEnv env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	CustomCallback cback;

	void setupOptions();
	void setupObjectiveFunction();
	void setupConstraints();

	vector<int> parseSchedule() const;
};
