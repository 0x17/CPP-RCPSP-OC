#pragma once

#include "../ProjectWithOvertime.h"
#include "../BasicSolverParameters.h"
#include <boost/optional.hpp>
#include <localsolver.h>

namespace Utils {
	class Tracer;
}

class ProjectNativeFunction : public localsolver::LSNativeFunction {
protected:
	ProjectWithOvertime &p;
	Utils::Tracer *tr = nullptr;
	localsolver::lsdouble bks = 0.0;
public:
	explicit ProjectNativeFunction(ProjectWithOvertime &_p);
	~ProjectNativeFunction() override = default;
	virtual int varCount() = 0;
	void setTracer(Utils::Tracer *tr);
};

class BaseSchedulingNativeFunction : public ProjectNativeFunction {
protected:
	virtual boost::optional<SGSResult> coreComputation(const localsolver::LSNativeContext &context) = 0;
	localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;
public:
	explicit BaseSchedulingNativeFunction(ProjectWithOvertime &_p) : ProjectNativeFunction(_p) {}
	~BaseSchedulingNativeFunction() override = default;
};

class ListSchedulingNativeFunction : public BaseSchedulingNativeFunction {
public:
	explicit ListSchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	~ListSchedulingNativeFunction() override = default;

	virtual SGSResult decode(std::vector<int> &order, const localsolver::LSNativeContext &context) = 0;
	boost::optional<SGSResult> coreComputation(const localsolver::LSNativeContext& context) override;
};

class TopOrderChecker : public localsolver::LSNativeFunction {
	const Project &p;
public:
	explicit TopOrderChecker(const Project &_p) : p(_p) {}
	localsolver::lsdouble call(const localsolver::LSNativeContext& context) override;
};

struct LSModelOptions : JsonUtils::IJsonSerializable {
	bool enforceTopOrdering = false,
		 parallelSGS = false;
	int partitionSize = 8;

	explicit LSModelOptions(bool enforce_top_ordering, bool parallel_sgs, int _partitionSize) : enforceTopOrdering(enforce_top_ordering), parallelSGS(parallel_sgs), partitionSize(_partitionSize) {}
	LSModelOptions() = default;
	virtual ~LSModelOptions() = default;

	//void fromJsonStr(const std::string &s);
	//void parseFromJsonFile(const std::string &fn = "LSModelOptions.json");

	json11::Json to_json() const override;
	void from_json(const json11::Json &obj) override;
};

class LSBaseModel : public ISolver {
protected:
	ProjectWithOvertime &p;
	localsolver::LocalSolver ls;
	std::unique_ptr<BaseSchedulingNativeFunction> decoder;

	virtual void addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;
	virtual std::vector<int> parseScheduleFromSolution(localsolver::LSSolution &sol) = 0;
	virtual void buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) = 0;

	virtual void applyInitialSolution() = 0;

	static LSModelOptions options;

public:
	LSBaseModel(ProjectWithOvertime &_p, BaseSchedulingNativeFunction *_decoder) : p(_p), decoder(_decoder) {}
	~LSBaseModel() override = default;

	std::vector<int> solve(SolverParams params) override;

	static LSModelOptions &getOptions() { return options;  }

	static std::string traceFilenameForLSModel(const std::string& outPath, int lsIndex, const std::string& instanceName);

private:
	void applyParams(SolverParams &params);
	void buildBaseModel();
};

class ListModel : public LSBaseModel {
protected:
	std::unique_ptr<TopOrderChecker> topOrderChecker;
	std::vector<localsolver::LSExpression> listElems;	
public:
	ListModel(ProjectWithOvertime &_p, ListSchedulingNativeFunction *_decoder);
	~ListModel() override = default;

private:
	localsolver::LSExpression activityList;

	void buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) override;
	void addTopologicalOrderingConstraint(localsolver::LSModel &model, localsolver::LSExpression &activityList) const;
	void applyInitialSolution() override;
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

class RandomKeySchedulingNativeFunction : public BaseSchedulingNativeFunction {
public:
	explicit RandomKeySchedulingNativeFunction(ProjectWithOvertime &_p) : BaseSchedulingNativeFunction(_p) {}
	~RandomKeySchedulingNativeFunction() override = default;
	virtual SGSResult decode(std::vector<float> &priorities, const localsolver::LSNativeContext &context) = 0;
	boost::optional<SGSResult> coreComputation(const localsolver::LSNativeContext &context) override;
};

class RandomKeyModel : public LSBaseModel {
protected:
	std::vector<localsolver::LSExpression> prioritiesElems;
public:
	RandomKeyModel(ProjectWithOvertime &_p, RandomKeySchedulingNativeFunction *_decoder);
	~RandomKeyModel() override = default;
private:
	void buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) override;
	virtual void applyInitialSolution() override;
};

