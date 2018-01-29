#include <iostream>
#include <string>

#include "Runners.h"

#include "GeneticAlgorithms/TimeWindow.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/FixedDeadline.h"
#include "LSModels/TimeWindowModels.h"
#include "LSModels/OvertimeBoundModels.h"
#include "LSModels/FixedDeadlineModels.h"
#include "GeneticAlgorithms/PartitionList.h"

using namespace std;

#define RUN_GA_FUNC_IMPL(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		Stopwatch sw; \
		sw.start(); \
		auto pair = ga.solve(); \
		double solvetime = sw.look(); \
		return{ pair.first, pair.second, solvetime, ga.getName() }; \
	}

namespace Runners {
	RunnerParams::RunnerParams(int _methodIndex, int _variant, double _timeLimit, int _iterLimit, bool _traceobj, const std::string& _outPath)
		: BasicSolverParameters(_timeLimit, _iterLimit, _traceobj, _outPath, 1),
		methodIndex(_methodIndex),
		variant(_variant) {}

	std::unique_ptr<ISolver> genListModelWithIndex(ProjectWithOvertime &p, int index, int variant) {
		std::unique_ptr<ISolver> solveModel = nullptr;
		switch (index) {
		default:
		case 0:
			ListBetaModel::setVariant(variant);
			solveModel = make_unique<ListBetaModel>(p);
			break;
		case 1:
			solveModel = make_unique<ListTauModel>(p);
			break;
		case 2:
			solveModel = make_unique<ListTauDiscreteModel>(p);
			break;
		case 3:
			solveModel = make_unique<ListFixedOvertimeModel>(p);
			break;
		case 4:
			solveModel = make_unique<ListDynamicOvertimeModel>(p);
			break;
		case 5:
			solveModel = make_unique<ListAlternativesModel>(p);
			break;
		case 6:
			solveModel = make_unique<GSListModel>(p);
			break;
		case 7:
			solveModel = make_unique<ListDeadlineModel>(p);
			break;
		case 8:
			solveModel = make_unique<RandomKeyFixedOvertimeModel>(p);
			break;
		case 9:
			solveModel = make_unique<RandomKeyDynamicOvertimeModel>(p);
			break;
		case 10:
			solveModel = make_unique<PartitionsModel>(p);
			break;			
		}
		return solveModel;
	}

	RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryDiscretizedGA, TimeWindowArbitraryDiscretizedGA)
	RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA)
	RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA)
	RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA)
	RUN_GA_FUNC_IMPL(runGoldenSectionSearchGA, GoldenSectionSearchGA)
	RUN_GA_FUNC_IMPL(runFixedDeadlineGA, FixedDeadlineGA)
	RUN_GA_FUNC_IMPL(runFixedCapaRandomKeyGA, FixedCapacityRandomKeyGA)
	RUN_GA_FUNC_IMPL(runTimeVaryCapaRandomKeyGA, TimeVaryingCapacityRandomKeyGA)
	RUN_GA_FUNC_IMPL(runOptimalSubschedulesGA, OptimalSubschedulesGA)
	RUN_GA_FUNC_IMPL(runPartitionListGA, PartitionListGA)
	
	vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = {
		runTwBorderGA,				// 0
		runTwArbitraryGA,			// 1
		runTwArbitraryDiscretizedGA,// 2
		runFixedCapaGA,				// 3
		runTimeVaryCapaGA,			// 4
		runCompAltsGA,				// 5
		runGoldenSectionSearchGA,	// 6
		runFixedDeadlineGA,			// 7
		runFixedCapaGA,				// 8
		runTimeVaryCapaRandomKeyGA, // 9
		runOptimalSubschedulesGA,   // 10
		runPartitionListGA			// 11
	};

	static vector<string> representationDescriptions = {
		"(lambda|beta)",
		"(lambda|tau)",
		"(lambda|tau-discrete)",
		"(lambda|zr)",
		"(lambda|zrt)",
		"(lambda) alts",
		"(lambda) gs",
		"(lambda|deadline-offset)",
		"(rk|zr)",
		"(rk|zrt)",
		"(lambda) sub",
		"(lambda) plist"
	};

	string getDescription(int ix) { return representationDescriptions[ix]; }

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index) {
		if (index < 0 || index >= funcs.size()) {
			throw std::runtime_error("Genetic algorithm index " + to_string(index) + " is out of range!");
		}
		auto gafunc = funcs[index];
		auto result = gafunc(p, params);
		LOG_I("Representation=" + result.name + " Profit=" + to_string(result.profit) + " Solvetime=" + to_string(result.solvetime));
		return result;
	}


	vector<int> runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, RunnerParams rparams) {
		GAParameters params;

		params.traceobj = rparams.traceobj;
		params.timeLimit = rparams.timeLimit;
		params.iterLimit = rparams.iterLimit;
		params.outPath = rparams.outPath;

		params.fitnessBasedPairing = false;
		params.rbbrs = true;
		params.fbiFeedbackInjection = false;
		params.enforceTopOrdering = true;
		
		params.numGens = -1;
		params.popSize = 80;
		params.pmutate = 5;
		params.partitionSize = 8;

		params.selectionMethod = SelectionMethod::BEST;
		params.sgs = ScheduleGenerationScheme::SERIAL;
		params.crossoverMethod = CrossoverMethod::OPC;

		params.fromJsonFile();

		if (rparams.methodIndex == 0)
			TimeWindowBordersGA::setVariant(rparams.variant);

		auto res = run(p, params, rparams.methodIndex);
		return res.sts;
	}

	vector<int> runLocalSolverModelWithIndex(ProjectWithOvertime& p, RunnerParams rparams) {
		unique_ptr<ISolver> lm = genListModelWithIndex(p, rparams.methodIndex, rparams.variant);
		SolverParams params(rparams.timeLimit, rparams.iterLimit);
		params.traceobj = rparams.traceobj;
		params.solverIx = rparams.methodIndex;
		params.outPath = rparams.outPath;

		auto &opts = ListModel::getOptions();
		opts.enforceTopOrdering = false;
		opts.parallelSGS = false;

		opts.parseFromJsonFile();

		return lm->solve(params);
	}

}
