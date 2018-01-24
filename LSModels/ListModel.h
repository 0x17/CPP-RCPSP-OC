#pragma once

#include "../ProjectWithOvertime.h"
#include "../BasicSolverParameters.h"
#include <localsolver.h>

namespace Utils {
	class Tracer;
}

class BaseSchedulingNativeFunction : public localsolver::LSNativeFunction {
protected:
	ProjectWithOvertime &p;
public:
	explicit BaseSchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
	~BaseSchedulingNativeFunction() override = default;
};

class SchedulingNativeFunction : public BaseSchedulingNativeFunction {
	Utils::Tracer *tr = nullptr;
	localsolver::lsdouble bks = 0.0;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	~SchedulingNativeFunction() override = default;

	virtual int varCount() = 0;
	virtual SGSResult decode(std::vector<int> &order, const localsolver::LSNativeContext &context) = 0;

	localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;

	void setTracer(Utils::Tracer *tr) { this->tr = tr; }
};

class TopOrderChecker : public localsolver::LSNativeFunction {
	const Project &p;
public:
	TopOrderChecker(const Project &_p) : p(_p) {}
	localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;
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

struct LSModelOptions {
	bool enforceTopOrdering, parallelSGS;
	LSModelOptions(bool enforce_top_ordering = false, bool parallel_sgs = false) : enforceTopOrdering(enforce_top_ordering), parallelSGS(parallel_sgs) {}

	void fromJsonStr(const std::string &s);
	void parseFromJsonFile(const std::string &fn = "LSModelOptions.json");
};

class ListModel : public ISolver {
protected:
	ProjectWithOvertime &p;
	localsolver::LocalSolver ls;
	SchedulingNativeFunction *decoder;
	TopOrderChecker *topOrderChecker;
	std::vector<localsolver::LSExpression> listElems;
	
	virtual void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;
	virtual std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) = 0;

	static LSModelOptions options;

public:
	ListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *_decoder);
	virtual ~ListModel();

	virtual std::vector<int> solve(SolverParams params);

	static std::string traceFilenameForListModel(const std::string& outPath, int lsIndex, const std::string& instanceName);

	static LSModelOptions &getOptions() { return options;  }

private:
	void buildModel();
	void applyParams(SolverParams &params);
	void addTopologicalOrderingConstraint(localsolver::LSModel &model, localsolver::LSExpression &activityList) const;
};

class TraceCallback : public localsolver::LSCallback {
    Utils::Tracer &tr;
    double secCtr;
public:
	explicit TraceCallback(Utils::Tracer &_tr);
	void callback(localsolver::LocalSolver &solver, localsolver::LSCallbackType type) override;
    ~TraceCallback() override = default;
};

//=================================================================================================

// TODO: Refactor random key codepath for reduced redundancy

class RandomKeySchedulingNativeFunction : public BaseSchedulingNativeFunction {
	Utils::Tracer *tr = nullptr;
	localsolver::lsdouble bks = 0.0;
public:
	explicit RandomKeySchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	~RandomKeySchedulingNativeFunction() override = default;

	virtual int varCount() = 0;
	virtual SGSResult decode(std::vector<float> &priorities, const localsolver::LSNativeContext &context) = 0;

	localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;

	void setTracer(Utils::Tracer *tr) { this->tr = tr; }
};

class RandomKeyModel : public ISolver {
protected:
	ProjectWithOvertime &p;
	localsolver::LocalSolver ls;
	RandomKeySchedulingNativeFunction *decoder;
	std::vector<localsolver::LSExpression> prioritiesElems;

	virtual void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;
	virtual std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) = 0;

public:
	RandomKeyModel(ProjectWithOvertime &_p, RandomKeySchedulingNativeFunction *_decoder);
	virtual ~RandomKeyModel();

	virtual std::vector<int> solve(SolverParams params);

	static std::string traceFilenameForRandomKeyModel(const std::string& outPath, int lsIndex, const std::string& instanceName);

private:
	void buildModel();
	void applyParams(SolverParams &params);
};
