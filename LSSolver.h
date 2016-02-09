//
// Created by André Schnabel on 24.10.15.
//

#ifndef CPP_RCPSP_OC_LSSOLVER_H
#define CPP_RCPSP_OC_LSSOLVER_H

#include <localsolver.h>

#include "Project.h"
#include "ProjectWithOvertime.h"

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

class ListAlternativesModel : public ListModel {
	class SerialSGSAlternativesDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSAlternativesDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		lsdouble call(const LSNativeContext& context) override;
	};

	vector<int> parseScheduleFromSolution(LSSolution &sol) override;
	void addAdditionalData(LSExpression &obj) override {}

public:
	ListAlternativesModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSAlternativesDecoder(_p)) {}
	virtual ~ListAlternativesModel() {}
};

class ListBetaModel : public ListModel {
	class SerialSGSBetaFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSBetaFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	vector<LSExpression> betaVar;

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListBetaModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSBetaFunction(_p)), betaVar(p.numJobs) {}
	virtual ~ListBetaModel() {}
};

class ListTauModel : public ListModel {
	class SerialSGSTauFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSTauFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	vector<LSExpression> tauVar;

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListTauModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSTauFunction(_p)), tauVar(p.numJobs) {}
	virtual ~ListTauModel() {}
};

class ListTauDiscreteModel : public ListModel {
	class SerialSGSIntegerFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSIntegerFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	static const lsint IV_COUNT = 4;
	vector<LSExpression> tauVar;

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

public:
	ListTauDiscreteModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSIntegerFunction(_p)), tauVar(p.numJobs) {}
	virtual ~ListTauDiscreteModel() {}
};

class ListFixedOvertimeModel : public ListModel {
	class SerialSGSZrDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	vector<LSExpression> zrVar;

public:
	ListFixedOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrDecoder(_p)), zrVar(p.numRes) {}
	virtual ~ListFixedOvertimeModel() {}
};

class ListDynamicOvertimeModel : public ListModel {
	class SerialSGSZrtDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSZrtDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	void addAdditionalData(LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	Matrix<LSExpression> zrtVar;

public:
	ListDynamicOvertimeModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSZrtDecoder(_p)), zrtVar(p.numRes, p.numPeriods) {}
	virtual ~ListDynamicOvertimeModel() {}
};

namespace LSSolver {
	vector<int> solve(ProjectWithOvertime &p, double timeLimit = 60.0, bool traceobj = false);
    vector<int> solveNative(ProjectWithOvertime &p);
	void writeLSPModelParamFile(ProjectWithOvertime &p, string outFilename);
};

#endif //CPP_RCPSP_OC_LSSOLVER_H
