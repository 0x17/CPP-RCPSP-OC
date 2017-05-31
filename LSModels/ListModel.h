#pragma once

#include "../ProjectWithOvertime.h"
#include <localsolver.h>

class BaseSchedulingNativeFunction : public localsolver::LSNativeFunction {
protected:
	ProjectWithOvertime &p;
public:
	explicit BaseSchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
	virtual ~BaseSchedulingNativeFunction() {}
};

class SchedulingNativeFunction : public BaseSchedulingNativeFunction {
	Utils::Tracer *tr = nullptr;
	localsolver::lsdouble bks = 0.0;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	virtual ~SchedulingNativeFunction() {}

	virtual int varCount() = 0;
	virtual SGSResult decode(std::vector<int> &order, const localsolver::LSNativeContext &context) = 0;

	virtual localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;

	void setTracer(Utils::Tracer *tr) { this->tr = tr; }
};

struct SolverParams : Utils::BasicSolverParameters {
	int seed, verbosityLevel, solverIx;
	explicit SolverParams(double _tlimit = -1.0, int _ilimit = -1);
};

class ListModel {
protected:
	ProjectWithOvertime &p;
	localsolver::LocalSolver ls;
	SchedulingNativeFunction *decoder;
	std::vector<localsolver::LSExpression> listElems;

	virtual void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;
	virtual std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) = 0;

public:
	ListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *_decoder);
	virtual ~ListModel();

	std::vector<int> solve(SolverParams params);

	static std::string traceFilenameForListModel(const std::string& outPath, int lsIndex, const std::string& instanceName);

private:
	void buildModel();
	void applyParams(SolverParams &params);
};

class TraceCallback : public localsolver::LSCallback {
    Utils::Tracer &tr;
    double secCtr;
public:
    TraceCallback(Utils::Tracer &_tr);
    virtual void callback(localsolver::LocalSolver &solver, localsolver::LSCallbackType type) override;
    virtual ~TraceCallback();
};
