//
// Created by André Schnabel on 06.11.15.
//

#ifndef CPP_RCPSP_OC_RUNNERS_H
#define CPP_RCPSP_OC_RUNNERS_H

#include "../ProjectWithOvertime.h"
#include "GeneticAlgorithm.h"
#include <vector>

#define RUN_GA_FUNC_SIGN(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params);

#define RUN_GA_FUNC_IMPL(funcname, gaType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		return runGeneticAlgorithm<gaType>(ga); \
	}

namespace GARunners {
    struct GAResult {
        vector<int> sts;
        float profit;
        double solvetime;
        string name;
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
    RUN_GA_FUNC_SIGN(runFixedCapaGA, FixedCapacityGA)
    RUN_GA_FUNC_SIGN(runTimeVaryCapaGA, TimeVaryingCapacityGA)
    RUN_GA_FUNC_SIGN(runCompAltsGA, CompareAlternativesGA)
    RUN_GA_FUNC_SIGN(runFixedDeadlineGA, FixedDeadlineGA)

    GAResult run(ProjectWithOvertime &p, GAParameters &params, int index);
}


#endif //CPP_RCPSP_OC_RUNNERS_H
