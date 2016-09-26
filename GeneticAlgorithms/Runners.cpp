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

    vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = {
		runTwBorderGA,				// 0
		runTwArbitraryGA,			// 1
		runTwArbitraryDiscretizedGA,// 2
		runFixedCapaGA,				// 3
		runTimeVaryCapaGA,			// 4
		runCompAltsGA,				// 5
		runGoldenCutSearchGA,		// 6
		runFixedDeadlineGA			// 7
	};

	vector<string> descriptions = { "(lambda|beta)", "(lambda|tau)", "(lambda|tau-discrete)", "(lambda|zr)", "(lambda|zrt)", "(lambda) alts", "(lambda) gs", "(lambda|deadline_offset)" };

	string getDescription(int ix) { return descriptions[ix];  }

	GAResult run(ProjectWithOvertime &p, GAParameters &params, int index) {
		if(index < 0 || index >= funcs.size()) {
			throw new runtime_error("Genetic algorithm index " + to_string(index) + " is out of range!");
		}
        auto gafunc = funcs[index];
        auto result = gafunc(p, params);
        cout << "Representation=" << result.name << " Profit=" << result.profit << " Solvetime=" << result.solvetime << endl;
        return result;
    }

	RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA)
	RUN_GA_FUNC_IMPL(runTwArbitraryDiscretizedGA, TimeWindowArbitraryDiscretizedGA)
	RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA)
	RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA)
	RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA)
	RUN_GA_FUNC_IMPL(runGoldenCutSearchGA, GoldenCutSearchGA)
    RUN_GA_FUNC_IMPL(runFixedDeadlineGA, FixedDeadlineGA)

}


