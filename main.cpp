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
	for (int i = 0; i < 3; i++) solMethods.push_back("LocalSolverNative" + to_string(i));
	cout << "Number of arguments must be >= 3" << endl;
	cout << "Usage: Solver SolutionMethod TimeLimitInSecs ProjectFileSM [traceobj]" << endl;
	cout << "Solution methods: " << endl;
	for (auto method : solMethods) cout << "\t" << method << endl;
}

int computeMinMaxMakespanDifference(ProjectWithOvertime &p) {
	int maxMs = p.makespan(p.serialSGS(p.topOrder));
	int minMs = p.makespan(p.serialSGS(p.topOrder, p.zmax).first);
	return maxMs - minMs;
}

void commandLineRunner(int argc, char * argv[]) {
    if(argc >= 4) {
        vector<int> sts;

        string solMethod = argv[1];
        double timeLimit = atof(argv[2]);
        ProjectWithOvertime p(argv[3]);
        string outFn = "";

        if(computeMinMaxMakespanDifference(p) <= 0) {
            cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
            return;
        }

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
			params.pmutate = 5;
            params.timeLimit = timeLimit;
            params.traceobj = traceobj;
            int gaIndex = stoi(solMethod.substr(2, 1));
            auto res = GARunners::run(p, params, gaIndex);
            sts = res.sts;
            outFn = "GA" + to_string(gaIndex) + "Results.txt";
		}
		else if (!solMethod.compare("LocalSolver")) {
			sts = LSSolver::solve(p, timeLimit, traceobj);
			outFn = "LocalSolverResults.txt";
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
			int lsnIndex = stoi(solMethod.substr(17, 1));
			vector<vector<int>(*)(int, ProjectWithOvertime&, double, bool)> lsfuncs = {
				LSSolver::solveListVarNative,
				LSSolver::solveListVarNative2,
				LSSolver::solveListVarNative3,
			};
			sts = lsfuncs[lsnIndex](0, p, timeLimit, traceobj);
			outFn = "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
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

void testFixedDeadlineHeuristic() {
	//string projFilename = "../../Projekte/j30/j301_1.sm";
	string projFilename = "QBWLBeispiel.DAT";
	//string projFilename = "MiniBeispiel.DAT";	
	ProjectWithOvertime p(projFilename);

	BranchAndBound bb(p);
	auto sts = bb.solve();

	//GAParameters params = { -1, 100, 5, 30, true, false };
	//auto res = GARunners::run(p, params, 5);
	//auto sts = res.sts;
	Utils::serializeSchedule(sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedule.txt");
}

void testLocalSolverNative(int seed) {
	string projFilename = "../../Projekte/j30/j301_1.sm";
	ProjectWithOvertime p(projFilename);
	auto sts = LSSolver::solveListVarNative3(seed ,p, 60.0);
	Utils::serializeSchedule(sts, "myschedule.txt");
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe ../../Projekte/j30/j301_1.sm myschedule.txt");
}

int main(int argc, char * argv[]) {
	commandLineRunner(argc, argv);
	//testFixedDeadlineHeuristic();
	//testLocalSolverNative(0 /*atoi(argv[1])*/);
    return 0;
}
