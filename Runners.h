#pragma once

#include <vector>
#include "GeneticAlgorithms/GeneticAlgorithm.h"
#include "Utils.h"

class ProjectWithOvertime;
class ListModel;

#define RUN_GA_FUNC_SIGN(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params);

#define RUN_GA_FUNC_IMPL(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		return runGeneticAlgorithm<gaType>(ga); \
	}

namespace Runners {
	struct RunnerParams : Utils::BasicSolverParameters {
		int methodIndex, variant;
		RunnerParams(int _methodIndex, int _variant, double _timeLimit, int _iterLimit, bool _traceobj, const string& _outPath);
	};

	ListModel *genListModelWithIndex(ProjectWithOvertime &p, int index, int variant = 0);

	struct GAResult {
		std::vector<int> sts;
		float profit;
		double solvetime;
		std::string name;
	};

	template<class T>
	GAResult runGeneticAlgorithm(T &ga) {
		Stopwatch sw;
		sw.start();
		auto pair = ga.solve();
		double solvetime = sw.look();
		return{ pair.first, pair.second, solvetime, ga.getName() };
	}

	RUN_GA_FUNC_SIGN(runTwBorderGA, TimeWindowBordersGA)
	RUN_GA_FUNC_SIGN(runTwArbitraryGA, TimeWindowArbitraryGA)
	RUN_GA_FUNC_SIGN(runTwArbitraryDiscretizedGA, TimeWindowArbitraryDiscretizedGA)
	RUN_GA_FUNC_SIGN(runFixedCapaGA, FixedCapacityGA)
	RUN_GA_FUNC_SIGN(runTimeVaryCapaGA, TimeVaryingCapacityGA)
	RUN_GA_FUNC_SIGN(runCompAltsGA, CompareAlternativesGA)
	RUN_GA_FUNC_SIGN(runGoldenCutSearchGA, GoldenCutSearchGA)
	RUN_GA_FUNC_SIGN(runFixedDeadlineGA, FixedDeadlineGA)

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index);

	std::string getDescription(int index);

	vector<int> runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, RunnerParams rparams);
	vector<int> runLocalSolverModelWithIndex(ProjectWithOvertime &p, RunnerParams rparams);
}