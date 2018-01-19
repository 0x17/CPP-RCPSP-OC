#pragma once

#include <vector>

#include "GeneticAlgorithms/GeneticAlgorithm.h"
#include "Utils.h"

class ProjectWithOvertime;
class ListModel;
class ISolver;

#define RUN_GA_FUNC_SIGN(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params);

#define RUN_GA_FUNC_IMPL(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		return runGeneticAlgorithm<gaType>(ga); \
	}

namespace Runners {
	struct RunnerParams : BasicSolverParameters {
		int methodIndex, variant;
		RunnerParams(int _methodIndex, int _variant, double _timeLimit, int _iterLimit, bool _traceobj, const std::string& _outPath);
	};

	std::unique_ptr<ISolver> genListModelWithIndex(ProjectWithOvertime &p, int index, int variant = 0);

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
	RUN_GA_FUNC_SIGN(runGoldenSectionSearchGA, GoldenSectionSearchGA)
	RUN_GA_FUNC_SIGN(runFixedDeadlineGA, FixedDeadlineGA)
	RUN_GA_FUNC_SIGN(runFixedCapaRandomKeyGA, FixedCapacityRandomKeyGA)
	RUN_GA_FUNC_SIGN(runTimeVaryCapaRandomKeyGA, TimeVaryingCapacityRandomKeyGA)

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index);

	std::string getDescription(int index);

	std::vector<int> runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, RunnerParams rparams);
	std::vector<int> runLocalSolverModelWithIndex(ProjectWithOvertime &p, RunnerParams rparams);

	enum RepresentationEnum {
		RE_LAMBDA_BETA,
		RE_LAMBDA_TAU,
		RE_LAMBDA_TAU_DISCRETE,
		RE_LAMBDA_ZR,
		RE_LAMBDA_ZRT,
		RE_LAMBDA_ALTS,
		RE_LAMBDA_GS,
		RE_LAMBDA_DEADLINE_OFFSET,
		RE_RANDKEY_ZRT
	};
}