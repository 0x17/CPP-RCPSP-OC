#pragma once

#include "ListModel.h"

class ListBetaModel : public ListModel {
	class SerialSGSBetaFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSBetaFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	vector<LSExpression> betaVar;

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	SchedulingNativeFunction* genDecoder() override {
		return new SerialSGSBetaFunction(p);
	}
public:
	ListBetaModel(ProjectWithOvertime &_p) : ListModel(_p), betaVar(p.numJobs) {}
	virtual ~ListBetaModel() {}
};

//==============================================================================================================

class ListTauModel : public ListModel {
	class SerialSGSTauFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSTauFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	vector<LSExpression> tauVar;

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	SchedulingNativeFunction* genDecoder() override {
		return new SerialSGSTauFunction(p);
	}
public:
	ListTauModel(ProjectWithOvertime &_p) : ListModel(_p), tauVar(p.numJobs) {}
	virtual ~ListTauModel() {}
};

//==============================================================================================================

class ListTauDiscreteModel : public ListModel {
	class SerialSGSIntegerFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSIntegerFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		virtual lsdouble call(const LSNativeContext& context) override;
	};

	static const lsint IV_COUNT = 4;
	vector<LSExpression> tauVar;

	void addAdditionalData(LSModel &model, LSExpression& obj) override;
	vector<int> parseScheduleFromSolution(LSSolution& sol) override;

	SchedulingNativeFunction* genDecoder() override {
		return new SerialSGSIntegerFunction(p);
	}
public:
	ListTauDiscreteModel(ProjectWithOvertime &_p) : ListModel(_p), tauVar(p.numJobs) {}
	virtual ~ListTauDiscreteModel() {}
};

//==============================================================================================================

class ListAlternativesModel : public ListModel {
	class SerialSGSAlternativesDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSAlternativesDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		lsdouble call(const LSNativeContext& context) override;
	};

	vector<int> parseScheduleFromSolution(LSSolution &sol) override;
	void addAdditionalData(LSModel &model, LSExpression &obj) override {}

	SchedulingNativeFunction* genDecoder() override {
		return new SerialSGSAlternativesDecoder(p);
	}
public:
	ListAlternativesModel(ProjectWithOvertime &_p) : ListModel(_p) {}
	virtual ~ListAlternativesModel() {}
};
