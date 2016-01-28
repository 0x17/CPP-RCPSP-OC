//
// Created by André Schnabel on 06.11.15.
//

#include <iostream>
#include <string>
#include "Runners.h"
#include "TimeWindow.h"
#include "OvertimeBound.h"
#include "FixedDeadline.h"

namespace GARunners {

    vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = { runTwBorderGA, runTwArbitraryGA, runFixedCapaGA, runTimeVaryCapaGA, runCompAltsGA, runFixedDeadlineGA };

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index) {
        auto gafunc = funcs[index];
        auto result = gafunc(p, params);
        cout << "Representation=" << result.name << " Profit=" << result.profit << " Solvetime=" << result.solvetime << endl;
        return result;
    }

    RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA)
    RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA)
    RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA)
    RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA)
    RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA)
    RUN_GA_FUNC_IMPL(runFixedDeadlineGA, FixedDeadlineGA)

}


