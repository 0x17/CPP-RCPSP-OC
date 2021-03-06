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
		bool saveTimeForBks;
		Options();
	};

	struct Result {
		std::vector<int> sts;
		bool optimal;
		Result(std::vector<int> &_sts, bool _optimal);
	};

	GurobiSolverBase(const ProjectWithOvertime& _p, Options _opts, bool _heuristicPeriodUB = true);
	virtual ~GurobiSolverBase() = default;

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

	class IncumbentCallback : public GRBCallback {
	public:
		IncumbentCallback();
		double getLastImprovementTime() const;

	private:
		void callback() override;

		Stopwatch sw;
		double previousBks, lastImprovementTime;
	};

	const ProjectWithOvertime &p;
	Options opts;
	const std::unique_ptr<GRBEnv> env;
	GRBModel model;
	Matrix<GRBVar> xjt, zrt;
	const std::unique_ptr<CustomCallback> cback;
	const std::unique_ptr<IncumbentCallback> incCback;
	bool heuristicPeriodUB;

	virtual void setupObjectiveFunction() = 0;
	virtual void setupConstraints() = 0;
	virtual void setupFeasibleMipStart() = 0;
	
	std::vector<int> parseSchedule() const;

	void mipStartFromSchedule(const std::vector<int> &sts);
};

class GurobiSolver : public GurobiSolverBase {
public:
	GurobiSolver(const ProjectWithOvertime& _p, Options _opts, boost::optional<const std::vector<int> &> _mipStartSts = boost::optional<const std::vector<int> &>());
	virtual ~GurobiSolver() = default;

private:
	void setupObjectiveFunction() override;
	void setupConstraints() override;
	void setupFeasibleMipStart() override;

	boost::optional<const std::vector<int>> mipStartSts;
};

class GurobiSubprojectSolver : public GurobiSolverBase {
public:
	GurobiSubprojectSolver(const ProjectWithOvertime& _p, Options& _opts);
	virtual ~GurobiSubprojectSolver() = default;

	void setupModelForSubproject(const std::vector<int>& sts, const std::vector<int>& nextPartition, bool forceInitIgnoredJobs = true);
	void supplyWithMIPStart(const std::vector<int> &sts, const std::vector<int> &nextPartition);

private:
	std::vector<GRBConstr> eachOnceConstraints;
	std::map<std::pair<int,int>,GRBConstr> precedenceConstraints;
	const std::vector<GRBVar> isms;
	
	void setupObjectiveFunction() override;
	void setupConstraints() override;
	void setupFeasibleMipStart() override;

	void fixJobToStartingTime(int j, int stj) const;
};

#endif