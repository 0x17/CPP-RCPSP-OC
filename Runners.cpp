#include <iostream>
#include <string>

#include "Runners.h"

#include "GeneticAlgorithms/TimeWindow.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/FixedDeadline.h"
#include "LSModels/TimeWindowModels.h"
#include "LSModels/OvertimeBoundModels.h"
#include "LSModels/FixedDeadlineModels.h"

using namespace std;

namespace Runners {
	RunnerParams::RunnerParams(int _methodIndex, int _variant, double _timeLimit, int _iterLimit, bool _traceobj, const std::string& _outPath)
		: BasicSolverParameters(_timeLimit, _iterLimit, _traceobj, _outPath, 1),
		methodIndex(_methodIndex),
		variant(_variant) {}

	std::unique_ptr<ListModel> genListModelWithIndex(ProjectWithOvertime &p, int index, int variant) {
		using std::make_unique;
		std::unique_ptr<ListModel> lm = nullptr;
		switch (index) {
		default:
		case 0:
			ListBetaModel::setVariant(variant);
			lm = make_unique<ListBetaModel>(p);
			break;
		case 1:
			lm = make_unique<ListTauModel>(p);
			break;
		case 2:
			lm = make_unique<ListTauDiscreteModel>(p);
			break;
		case 3:
			lm = make_unique<ListFixedOvertimeModel>(p);
			break;
		case 4:
			lm = make_unique<ListDynamicOvertimeModel>(p, false);
			break;
		case 5:
			lm = make_unique<ListAlternativesModel>(p);
			break;
		case 6:
			lm = make_unique<GSListModel>(p);
			break;
		case 7:
			lm = make_unique<ListDeadlineModel>(p);
			break;
		}
		return lm;
	}


	vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = {
		runTwBorderGA,				// 0
		runTwArbitraryGA,			// 1
		runTwArbitraryDiscretizedGA,// 2
		runFixedCapaGA,				// 3
		runTimeVaryCapaGA,			// 4
		runCompAltsGA,				// 5
		runGoldenSectionSearchGA,	// 6
		runFixedDeadlineGA			// 7
	};

	static vector<string> representationDescriptions = {
		"(lambda|beta)",
		"(lambda|tau)",
		"(lambda|tau-discrete)",
		"(lambda|zr)",
		"(lambda|zrt)",
		"(lambda) alts",
		"(lambda) gs",
		"(lambda|deadline-offset)"
	};

	string getDescription(int ix) { return representationDescriptions[ix]; }

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index) {
		if (index < 0 || index >= funcs.size()) {
			throw new std::runtime_error("Genetic algorithm index " + to_string(index) + " is out of range!");
		}
		auto gafunc = funcs[index];
		auto result = gafunc(p, params);
		LOG_I("Representation=" + result.name + " Profit=" + to_string(result.profit) + " Solvetime=" + to_string(result.solvetime));
		return result;
	}

	RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryDiscretizedGA, TimeWindowArbitraryDiscretizedGA)
	RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA)
	RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA)
	RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA)
	RUN_GA_FUNC_IMPL(runGoldenSectionSearchGA, GoldenSectionSearchGA)
	RUN_GA_FUNC_IMPL(runFixedDeadlineGA, FixedDeadlineGA)


	vector<int> runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, RunnerParams rparams) {
		GAParameters params;
		params.fitnessBasedPairing = false;
		params.numGens = -1;
		params.popSize = 80;
		params.pmutate = 5;
		params.timeLimit = rparams.timeLimit;
		params.iterLimit = rparams.iterLimit;
		params.traceobj = rparams.traceobj;
		params.selectionMethod = SelectionMethod::BEST;
		params.rbbrs = true;
		params.outPath = rparams.outPath;

		params.fromJsonFile();

		if (rparams.methodIndex == 0)
			TimeWindowBordersGA::setVariant(rparams.variant);

		auto res = run(p, params, rparams.methodIndex);
		return res.sts;
	}

	vector<int> runLocalSolverModelWithIndex(ProjectWithOvertime& p, RunnerParams rparams) {
		unique_ptr<ListModel> lm = genListModelWithIndex(p, rparams.methodIndex, rparams.variant);
		SolverParams params(rparams.timeLimit, rparams.iterLimit);
		params.traceobj = rparams.traceobj;
		params.solverIx = rparams.methodIndex;
		params.outPath = rparams.outPath;
		return lm->solve(params);
	}

}
