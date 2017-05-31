#pragma once

#include "ListModel.h"

class ListBetaModel : public ListModel {
	class SerialSGSBetaFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSBetaFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;

	std::vector<localsolver::LSExpression> betaVar;
	static ProjectWithOvertime::BorderSchedulingOptions options;

public:
	ListBetaModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSBetaFunction(_p)), betaVar(p.numJobs) {}
	virtual ~ListBetaModel() {}

	static void setVariant(int variant);
};

//==============================================================================================================

class ListTauModel : public ListModel {
	class SerialSGSTauFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSTauFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	std::vector<localsolver::LSExpression> tauVar;

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;
public:
	ListTauModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSTauFunction(_p)), tauVar(p.numJobs) {}
	virtual ~ListTauModel() {}
};

//==============================================================================================================

class ListTauDiscreteModel : public ListModel {
	class SerialSGSIntegerFunction : public SchedulingNativeFunction {
	public:
		explicit SerialSGSIntegerFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	static const localsolver::lsint IV_COUNT = 4;
	std::vector<localsolver::LSExpression> tauVar;

	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression& obj) override;
	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution& sol) override;
public:
	ListTauDiscreteModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSIntegerFunction(_p)), tauVar(p.numJobs) {}
	virtual ~ListTauDiscreteModel() {}
};

//==============================================================================================================

class ListAlternativesModel : public ListModel {
	class SerialSGSAlternativesDecoder : public SchedulingNativeFunction {
	public:
		explicit SerialSGSAlternativesDecoder(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
		int varCount() override;
		SGSResult decode(std::vector<int>& order, const localsolver::LSNativeContext& context) override;
	};

	std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) override;
	void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) override {}
public:
	ListAlternativesModel(ProjectWithOvertime &_p) : ListModel(_p, new SerialSGSAlternativesDecoder(_p)) {}
	virtual ~ListAlternativesModel() {}
};
