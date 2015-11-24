//
// Created by André Schnabel on 06.11.15.
//

#include <iostream>
#include <string>
#include <sstream>
#include "Runners.h"
#include "TimeWindow.h"
#include "OvertimeBound.h"
#include "FixedDeadline.h"

namespace GARunners {

    vector<string> gaNames = { "TwoBorderGA", "TWArbitraryGA", "FixedCapaGA", "TimeVaryCapaGA", "CompAltsGA", "RunFixedDeadlineGA" };
    vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = { runTwBorderGA, runTwArbitraryGA, runFixedCapaGA, runTimeVaryCapaGA, runCompAltsGA, runFixedDeadlineGA };

    void batchRunSpecific(const string path, GAParameters &params, int index) {
		cout << "Solving instances in " << path << " with " << gaNames[index] << endl;
        auto instanceFilenames = Utils::filenamesInDirWithExt(path, ".sm");
		stringstream outStr;
        for(auto instanceFn : instanceFilenames) {
            ProjectWithOvertime p(instanceFn);
            auto res = runSpecific(p, params, index);
            outStr << instanceFn << ";" << to_string(res.profit) << endl;

        }
		Utils::spit(outStr.str(), gaNames[index] + "_Results.csv");
    }

    void batchRunAll(const string path, GAParameters &params) {
        for(int i=0; i<funcs.size(); i++)
            batchRunSpecific(path, params, i);
    }

	void batchRunRange(const string path, GAParameters& params, int startIx, int endIx) {
		for(int i = startIx; i <= endIx; i++)
			batchRunSpecific(path, params, i);
	}

	GAResult runSpecific(ProjectWithOvertime &p, GAParameters &params, int index) {
        auto gafunc = funcs[index];
        auto result = gafunc(p, params);
        cout << endl << "Representation=" << result.name << " Profit=" << result.profit << " Solvetime=" << result.solvetime << endl;
        return result;
    }

    void runRange(ProjectWithOvertime &p, GAParameters &params, int startIx, int endIx) {
        for(int i=startIx; i<=endIx; i++)
            runSpecific(p, params, i);
    }

    void runAll(ProjectWithOvertime &p, GAParameters &params) {
        for(int i=0; i<funcs.size(); i++)
            runSpecific(p, params, i);
    }

    RUN_GA_FUNC_IMPL(runTwBorderGA, TimeWindowBordersGA, LambdaBeta)
    RUN_GA_FUNC_IMPL(runTwArbitraryGA, TimeWindowArbitraryGA, LambdaTau)
    RUN_GA_FUNC_IMPL(runFixedCapaGA, FixedCapacityGA, LambdaZr)
    RUN_GA_FUNC_IMPL(runTimeVaryCapaGA, TimeVaryingCapacityGA, LambdaZrt)
    RUN_GA_FUNC_IMPL(runCompAltsGA, CompareAlternativesGA, vector<int>)
    RUN_GA_FUNC_IMPL(runFixedDeadlineGA, FixedDeadlineGA, DeadlineLambda)

}


