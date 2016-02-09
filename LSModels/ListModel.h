#ifndef __LIST_MODEL_H__
#define __LIST_MODEL_H__

#include "../ProjectWithOvertime.h"
#include <localsolver.h>

using namespace localsolver;

class SchedulingNativeFunction : public LSNativeFunction {
protected:
	ProjectWithOvertime &p;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
	virtual ~SchedulingNativeFunction() {}
};

struct SolverParams {
	int seed;
	double timeLimit;
	int threadCount;
	int verbosityLevel;

	SolverParams(double _tlimit)
		: seed(0), timeLimit(_tlimit), threadCount(1), verbosityLevel(2)
	{}
};

class ListModel {
protected:
	ProjectWithOvertime &p;

	LocalSolver ls;
	LSModel model;

	vector<LSExpression> listElems;

	SchedulingNativeFunction *decoder;

	virtual void addAdditionalData(LSExpression &obj) = 0;
	virtual vector<int> parseScheduleFromSolution(LSSolution &sol) = 0;

public:
	ListModel(ProjectWithOvertime &_p, SchedulingNativeFunction *_decoder)
		: p(_p), model(ls.getModel()), listElems(p.numJobs), decoder(_decoder) {}
	virtual ~ListModel();

	vector<int> solve(SolverParams params);

private:
	void buildModel();
	void applyParams(SolverParams &params);
};

#endif
