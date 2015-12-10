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

void showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver" };
	for (int i = 0; i < 5; i++) solMethods.push_back("GA" + to_string(i));
	cout << "Number of arguments must be >= 3" << endl;
	cout << "Usage: Solver SolutionMethod TimeLimitInSecs ProjectFileSM [traceobj]" << endl;
	cout << "Solution methods: " << endl;
	for (auto method : solMethods) cout << "\t" << method << endl;
}

void commandLineRunner(int argc, const char * argv[]) {
    if(argc >= 4) {
        vector<int> sts;

        string solMethod = argv[1];
        double timeLimit = atof(argv[2]);
        ProjectWithOvertime p(argv[3]);
        string outFn = "";

        bool traceobj = (argc == 5 && !string("traceobj").compare(argv[4]));

        if(!solMethod.compare("BranchAndBound")) {
            BranchAndBound b(p, timeLimit);
            sts = b.solve(false, traceobj);
            outFn = "BranchAndBoundResults.txt";
        } else if(boost::starts_with(solMethod, "GA")) {
            GAParameters params;
            params.fitnessBasedPairing = true;
            params.numGens = -1;
            params.popSize = 80;
            params.timeLimit = timeLimit;
            params.traceobj = traceobj;
            int gaIndex = stoi(solMethod.substr(2, 1));
            auto res = GARunners::run(p, params, gaIndex);
            sts = res.sts;
            outFn = "GA" + to_string(gaIndex) + "Results.txt";
        } else if(!solMethod.compare("LocalSolver")) {
			sts = LSSolver::solve(p, timeLimit, traceobj);
			outFn = "LocalSolverResults.txt";
        } else {
			throw runtime_error("Unknown method: " + solMethod + "!");
        }

        if(!traceobj) {
            string resStr = (sts[0] == -1) ? "infes" : to_string(p.calcProfit(sts));
            Utils::spitAppend(string(argv[3])+";"+resStr+"\n", outFn);
        }
	}
	else showUsage();
}

int main(int argc, const char * argv[]) {
	//string projFilename = "../../Projekte/j30/j3010_9.sm";
	//string projFilename = "QBWLBeispiel.DAT";
	//ProjectWithOvertime p(projFilename);
	/*auto sts = LSSolver::solve(p);
	Utils::serializeSchedule(sts, "myschedulebiatch.txt");
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");*/

    commandLineRunner(argc, argv);

    return 0;
}
