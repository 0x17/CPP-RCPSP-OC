#ifndef __LIST_MODEL_H__
#define __LIST_MODEL_H__

#include "../ProjectWithOvertime.h"
#include <localsolver.h>

using namespace localsolver;

class BaseSchedulingNativeFunction : public LSNativeFunction {
protected:
	ProjectWithOvertime &p;
public:
	explicit BaseSchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
	virtual ~BaseSchedulingNativeFunction() {}
};

class SchedulingNativeFunction : public BaseSchedulingNativeFunction {
	Utils::Tracer *tr = nullptr;
	lsdouble bks = 0.0;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	virtual ~SchedulingNativeFunction() {}

	virtual int varCount() = 0;
	virtual SGSResult decode(vector<int> &order, const LSNativeContext &context) = 0;

	virtual lsdouble call(const LSNativeContext& context) override;

	void setTracer(Utils::Tracer *tr) { this->tr = tr; }
};

struct SolverParams : Utils::BasicSolverParameters {
	int seed, verbosityLevel, solverIx;
	explicit SolverParams(double _tlimit = -1.0, int _ilimit = -1);
};

class ListModel {
protected:
	ProjectWithOvertime &p;
	LocalSolver ls;
	SchedulingNativeFunction *decoder;
	vector<LSExpression> listElems;

	virtual void addAdditionalData(LSModel &model, LSExpression &obj) = 0;
	virtual vector<int> parseScheduleFromSolution(LSSolution &sol) = 0;

public:
	ListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *_decoder);
	virtual ~ListModel();

	vector<int> solve(SolverParams params);

	static string traceFilenameForListModel(const string& outPath, int lsIndex, const string& instanceName);

private:
	void buildModel();
	void applyParams(SolverParams &params);
};

class TraceCallback : public LSCallback {
    Utils::Tracer &tr;
    double secCtr;
public:
    TraceCallback(Utils::Tracer &_tr);
    virtual void callback(LocalSolver &solver, LSCallbackType type) override;
    virtual ~TraceCallback();
};

#endif
