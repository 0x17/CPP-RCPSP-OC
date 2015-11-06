//
// Created by André Schnabel on 06.11.15.
//

#include "Runners.h"
#include <iostream>
namespace GARunners {

    vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = { runTwBorderGA, runTwArbitraryGA, runFixedCapaGA, runTimeVaryCapaGA, runCompAltsGA };

    void runAll(ProjectWithOvertime &p, GAParameters params) {
        for(auto gafunc : funcs) {
            auto result = gafunc(p, params);
            cout << "Representation=" << result.name << " Profit=" << result.profit << " Solvetime=" << result.solvetime << endl;
        }
    }

    RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA, LambdaBeta)
    RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA, LambdaTau)
    RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA, LambdaZr)
    RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA, LambdaZrt)
    RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA, vector<int>);

}


