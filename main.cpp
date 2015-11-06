#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "ScheduleVisualizer.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/TimeWindow.h"
#include "Stopwatch.h"

#define RUN_GA_FUNC(funcname, gaType, indivType) \
	GAResult funcname(ProjectWithOvertime &p) { \
		gaType ga(p); \
		return runGeneticAlgorithm<indivType>(ga); \
	}

namespace GARunners {
	struct GAResult {
		vector<int> sts;
		float profit;
		double solvetime;
	};

	template<class T>
	GAResult runGeneticAlgorithm(GeneticAlgorithm<T> &ga) {
		Stopwatch sw;
		sw.start();
		auto pair = ga.solve();
		double solvetime = sw.look();
		return{ pair.first, pair.second, solvetime };
	}

	RUN_GA_FUNC(runTwBorderGA, TimeWindowBordersGA, LambdaBeta)
	RUN_GA_FUNC(runTwArbitraryGA, TimeWindowArbitraryGA, LambdaTau)
	RUN_GA_FUNC(runFixedCapaGA, FixedCapacityGA, LambdaZr)
	RUN_GA_FUNC(runTimeVaryCapaGA, TimeVaryingCapacityGA, LambdaZrt)
	RUN_GA_FUNC(runCompAltsGA, CompareAlternativesGA, vector<int>);

	vector<GAResult(*)(ProjectWithOvertime &)> funcs = { runTwBorderGA, runTwArbitraryGA, runFixedCapaGA, runTimeVaryCapaGA, runCompAltsGA };
}

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("QBWLBeispiel.DAT");
	GARunners::GAResult result = GARunners::runCompAltsGA(p);
	cout << endl << "Solvetime = " << result.solvetime << endl;

	/*if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, "LSInstance.txt");
	}*/
    //auto optimalSts = LSSolver::solveMIPStyle(p);
    //auto optimalSts = LSSolver::solve(p);
	//auto optimalSts = LSSolver::solve2(p);
	//auto optimalSts = LSSolver::solve3(p);
	Utils::serializeSchedule(result.sts, "myschedulebiatch.txt");
    //vector<int> optimalSts(p.numJobs);
    //ScheduleVisualizer::drawScheduleToPDF(p, optimalSts, "out.pdf");
	//cin.get();
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");
    return 0;
}
