#pragma once

#ifndef DISABLE_GUROBI

#include <vector>
#include <map>
#include <gurobi_c++.h>

#include "Matrix.h"
#include "Utils.h"
#include "Logger.h"
#include "BasicSolverParameters.h"

class ProjectWithOvertime;

class GurobiSolverBase {
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

	GurobiSolverBase(const ProjectWithOvertime& _p, Options _opts);
	virtual ~GurobiSolverBase() = default;

	void restrictJobToTimeWindow(int j, int eft, int lft);

	void relaxJob(int j);
	void relaxAllJobs();

	void buildModel();

	Result solve();	

	static std::string traceFilenameForInstance(const std::string& outPath, const std::string& instanceName);

	static std::unique_ptr<GRBEnv> setupOptions(Options opts);

protected:
	class CustomCallback : public GRBCallback {
	public:
		CustomCallback(const std::string &outPath, const std::string& instanceName);
		void manualCallback(float bks);

	private:
		void callback() override;
		Utils::Tracer tr;
		Stopwatch sw;
	};

	const ProjectWithOvertime &p;
	Options opts;
	const std::unique_ptr<GRBEnv> env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	const std::unique_ptr<CustomCallback> cback;

	virtual void setupObjectiveFunction() = 0;
	virtual void setupConstraints() = 0;
	virtual void setupFeasibleMipStart() = 0;
	
	std::vector<int> parseSchedule() const;
};

class GurobiSolver : public GurobiSolverBase {
public:
	GurobiSolver(const ProjectWithOvertime& _p, Options _opts) : GurobiSolverBase(_p, _opts) {}
	virtual ~GurobiSolver() = default;

private:
	void setupObjectiveFunction() override;
	void setupConstraints() override;
	void setupFeasibleMipStart() override;	
};

class GurobiSubprojectSolver : public GurobiSolverBase {
public:
	GurobiSubprojectSolver(const ProjectWithOvertime& _p, Options& _opts);
	virtual ~GurobiSubprojectSolver() = default;

	void setupModelForSubproject(const std::vector<int>& _sts, const std::vector<int>& _nextPartition);

private:
	std::vector<GRBConstr> eachOnceConstraints;
	std::map<std::pair<int,int>,GRBConstr> precedenceConstraints;
	const std::vector<GRBVar> isms;
	
	void setupObjectiveFunction() override;
	void setupConstraints() override;
	void setupFeasibleMipStart() override;
};

#endif