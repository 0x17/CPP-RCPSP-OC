//
// Created by André Schnabel on 06.11.15.
//

#ifndef CPP_RCPSP_OC_RUNNERS_H
#define CPP_RCPSP_OC_RUNNERS_H

#include "../ProjectWithOvertime.h"
#include "GeneticAlgorithm.h"
#include "TimeWindow.h"
#include "OvertimeBound.h"
#include "FixedDeadline.h"
#include <vector>

#define RUN_GA_FUNC_SIGN(funcname, gaType, indivType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params);

#define RUN_GA_FUNC_IMPL(funcname, gaType, indivType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		return runGeneticAlgorithm<indivType>(ga, #indivType); \
	}

namespace GARunners {
    struct GAResult {
        std::vector<int> sts;
        float profit;
        double solvetime;
        string name;
    };

    template<class T>
    GAResult runGeneticAlgorithm(GeneticAlgorithm<T> &ga, string name) {
        Stopwatch sw;
        sw.start();
        auto pair = ga.solve();
        double solvetime = sw.look();
        return{ pair.first, pair.second, solvetime, name };
    }

    RUN_GA_FUNC_SIGN(runTwBorderGA, TimeWindowBordersGA, LambdaBeta)
    RUN_GA_FUNC_SIGN(runTwArbitraryGA, TimeWindowArbitraryGA, LambdaTau)
    RUN_GA_FUNC_SIGN(runFixedCapaGA, FixedCapacityGA, LambdaZr)
    RUN_GA_FUNC_SIGN(runTimeVaryCapaGA, TimeVaryingCapacityGA, LambdaZrt)
    RUN_GA_FUNC_SIGN(runCompAltsGA, CompareAlternativesGA, vector<int>)
    RUN_GA_FUNC_SIGN(runFixedDeadlineGA, FixedDeadlineGA, DeadlineLambda)

    void runAll(ProjectWithOvertime &p, GAParameters &params);
    GAResult runSpecific(ProjectWithOvertime &p, GAParameters &params, int index);
    void runRange(ProjectWithOvertime &p, GAParameters &params, int startIx, int endIx);
}


#endif //CPP_RCPSP_OC_RUNNERS_H
