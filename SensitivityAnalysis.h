//
// Created by André Schnabel on 10.10.18.
//

#pragma once

#include <string>
#include <vector>
#include "ProjectWithOvertime.h"
#include "BasicSolverParameters.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GurobiSolver.h"

namespace sensitivity {

	struct ResultForValue {
		float value;
		std::vector<int> sts;
	};

	template<class Modifier>
	std::list<ResultForValue> collectResultsForRange(Modifier modifier, const std::string &fn, float from, float to,
													 float step) {
		ProjectWithOvertime p(fn);
		std::list<ResultForValue> res;

		GAParameters params;
		params.timeLimit = -1;
		params.iterLimit = 500;
		params.numGens = -1;
		params.fitnessBasedPairing = false;
		params.rbbrs = true;
		params.fbiFeedbackInjection = false;
		params.enforceTopOrdering = true;
		params.traceobj = false;

		GurobiSolver::Options opts;
		opts.timeLimit = GRB_INFINITY;
		opts.threadCount = 0;
		opts.saveTimeForBks = false;
		opts.traceobj = false;

		float v = from;
		while (v <= to) {
			modifier(&p, v);
			//FixedCapacityGA ga(p);
			//ga.setParameters(params);
			//res.push_back({v, ga.solve().first});

			GurobiSolver gsolver(p, opts);
			gsolver.buildModel();
			res.push_back({v, gsolver.solve().sts});
			v += step;
		}
		return res;
	}

	void varyTotalAvailableCapacity(const std::string &fn, int r);

	void resultsToCsvFile(const ProjectWithOvertime &p, const std::list<ResultForValue> &results, const std::string &ofn);

}