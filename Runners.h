#pragma once

#include <vector>

#include "GeneticAlgorithms/GeneticAlgorithm.h"
#include "Utils.h"

class ProjectWithOvertime;
class ListModel;
class ISolver;

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
		RE_RANDKEY_ZR,
		RE_RANDKEY_ZRT,
		RE_LAMBDA_SUB,
		RE_LAMBDA_PLIST
	};
}