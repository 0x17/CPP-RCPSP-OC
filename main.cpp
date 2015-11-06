#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "ScheduleVisualizer.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/TimeWindow.h"
#include "Stopwatch.h"

#define RUN_GA_FUNC(funcname, gaType, indivType) \
	GAResult funcname(ProjectWithOvertime &p, GAParameters &params) { \
		gaType ga(p); \
        ga.setParameters(params); \
		return runGeneticAlgorithm<indivType>(ga, #indivType); \
	}

namespace GARunners {
	struct GAResult {
		vector<int> sts;
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

	RUN_GA_FUNC(runTwBorderGA, TimeWindowBordersGA, LambdaBeta)
	RUN_GA_FUNC(runTwArbitraryGA, TimeWindowArbitraryGA, LambdaTau)
	RUN_GA_FUNC(runFixedCapaGA, FixedCapacityGA, LambdaZr)
	RUN_GA_FUNC(runTimeVaryCapaGA, TimeVaryingCapacityGA, LambdaZrt)
	RUN_GA_FUNC(runCompAltsGA, CompareAlternativesGA, vector<int>);

	vector<GAResult(*)(ProjectWithOvertime &, GAParameters &)> funcs = { runTwBorderGA, runTwArbitraryGA, runFixedCapaGA, runTimeVaryCapaGA, runCompAltsGA };
    
    void runAll(ProjectWithOvertime &p, GAParameters params) {
        for(auto gafunc : funcs) {
            auto result = gafunc(p, params);
            cout << "Representation=" << result.name << " Profit=" << result.profit << " Solvetime=" << result.solvetime << endl;
        }
    }
}

void convertArgFileToLSP(int argc, char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, "LSInstance.txt");
	}
}

int main(int argc, const char * argv[]) {
    GAParameters params;
    params.popSize = 80;
    params.numGens = 100;
    params.pmutate = 5;
    params.timeLimit = -1.0;

    ProjectWithOvertime p("QBWLBeispiel.DAT");
    GARunners::runAll(p, params);

	//Utils::serializeSchedule(result.sts, "myschedulebiatch.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");
    return 0;
}
