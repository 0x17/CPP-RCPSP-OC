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
    if(argc == 4) {
        vector<int> sts;

        string solMethod = argv[1];
        double timeLimit = atof(argv[2]);
        ProjectWithOvertime p(argv[3]);
        string outFn = "";

        if(!solMethod.compare("BranchAndBound")) {
            BranchAndBound b(p, timeLimit);
            sts = b.solve(false);
            outFn = "BranchAndBoundResults.txt";
        } else if(boost::starts_with(solMethod, "GA")) {
            GAParameters params;
            params.fitnessBasedPairing = true;
            params.numGens = -1;
            params.popSize = 80;
            params.timeLimit = timeLimit;
            int gaIndex = stoi(solMethod.substr(2, 1));
            auto res = GARunners::runSpecific(p, params, gaIndex);
            sts = res.sts;
            outFn = "GA" + to_string(gaIndex) + "Results.txt";
        } else if(!solMethod.compare("LocalSolver")) {
			sts = LSSolver::solve(p, timeLimit);
			outFn = "LocalSolverResults.txt";
        } else {
			throw runtime_error("Unknown method: " + solMethod + "!");
        }

		string resStr = (sts[0] == -1) ? "infes" : to_string(p.calcProfit(sts));
        Utils::spitAppend(string(argv[3])+";"+resStr, outFn);
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
