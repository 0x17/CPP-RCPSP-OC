#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/Runners.h"
#include "BranchAndBound.h"
#include <boost/algorithm/string.hpp>

void convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

void commandLineRunner(int argc, const char * argv[]) {
    if(argc == 3) {
        vector<int> sts;

        ProjectWithOvertime p(argv[2]);
        string solMethod = argv[1];
        string outFn = "";

        if(boost::equal(solMethod, "BranchAndBound")) {
            BranchAndBound b(p);
            sts = b.solve(true);
            outFn = "BranchAndBoundResults.txt";
        } else if(boost::starts_with(solMethod, "GA")) {
            GAParameters params;
            params.fitnessBasedPairing = true;
            params.numGens = 100;
            params.popSize = 80;
            params.timeLimit = 60;
            int gaIndex = stoi(solMethod.substr(2, 1));
            auto res = GARunners::runSpecific(p, params, gaIndex);
            sts = res.sts;
            outFn = "GA" + to_string(gaIndex) + "Results.txt";
        } else if(boost::equal(solMethod, "LocalSolver")) {
			sts = LSSolver::solve(p);
			outFn = "LocalSolverResults.txt";
        }

        Utils::spitAppend(string(argv[2])+";"+to_string(p.calcProfit(sts)), outFn);
    }
}

int main(int argc, const char * argv[]) {
	//string projFilename = "../../Projekte/j30/j301_1.sm";
	/*string projFilename = "QBWLBeispiel.DAT";
	ProjectWithOvertime p(projFilename);
	auto sts = LSSolver::solve(p);
	Utils::serializeSchedule(sts, "myschedulebiatch.txt");
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");*/

    commandLineRunner(argc, argv);

    return 0;
}
