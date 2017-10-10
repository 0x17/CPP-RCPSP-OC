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
protected:
	bool enforceTopOrdering;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p, bool _enforceTopOrdering = false) : BaseSchedulingNativeFunction(_p), enforceTopOrdering(_enforceTopOrdering) {}
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

class ListModel {
protected:
	ProjectWithOvertime &p;
	localsolver::LocalSolver ls;
	SchedulingNativeFunction *decoder;
	TopOrderChecker *topOrderChecker;
	std::vector<localsolver::LSExpression> listElems;
	bool enforceTopOrdering;

	virtual void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;
	virtual std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) = 0;

public:
	ListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *_decoder, bool _enforceTopOrdering = false);
	virtual ~ListModel();

	std::vector<int> solve(SolverParams params);

	static std::string traceFilenameForListModel(const std::string& outPath, int lsIndex, const std::string& instanceName);

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
