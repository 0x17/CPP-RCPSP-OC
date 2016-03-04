#include "ProjectWithOvertime.h"
#include "LSModels/NaiveModels.h"
#include "LSModels/OvertimeBoundModels.h"
#include "LSModels/TimeWindowModels.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/Runners.h"
#include "BranchAndBound.h"
#include <boost/algorithm/string.hpp>
#include <cmath>

void convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

void showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver" };
	for (int i = 0; i < 5; i++) solMethods.push_back("GA" + to_string(i));
	for (int i = 0; i < 6; i++) solMethods.push_back("LocalSolverNative" + to_string(i));
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
			params.selectionMethod = SelectionMethod::BEST;
            params.rbbrs = true;
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
			ListModel *lm;
			switch(lsnIndex) {
			default:
			case 0:
				lm = new ListBetaModel(p);
				break;
			case 1:
				lm = new ListTauModel(p);
				break;
			case 2:
				lm = new ListTauDiscreteModel(p);
				break;
            case 3:
                lm = new ListAlternativesModel(p);
                break;
			case 4:
				lm = new ListFixedOvertimeModel(p);
				break;
			case 5:
				lm = new ListDynamicOvertimeModel(p);
				break;
			}
			SolverParams params(timeLimit);
            params.trace = traceobj;
            params.solverIx = lsnIndex;
			sts = lm->solve(params);
			delete lm;
			outFn = "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
        } else {
			throw runtime_error("Unknown method: " + solMethod + "!");
        }

        if(!traceobj) {
            string resStr = (sts[0] == -1) ? "infes" : to_string(p.calcProfit(sts));
            Utils::spitAppend(string(argv[3])+";"+resStr+"\n", outFn);
        }

		Utils::serializeSchedule(sts, "myschedule.txt");
		Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
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
	string projFilename = "../../Projekte/j30/j3010_1.sm";
	ProjectWithOvertime p(projFilename);
	ListAlternativesModel lm(p);
	SolverParams params(15.0);
	params.seed = seed;
	auto sts = lm.solve(params);
	Utils::serializeSchedule(sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe ../../Projekte/j30/j301_1.sm myschedule.txt");
}

void benchmarkGeneticAlgorithm(int gaIndex, int iterLimit) {
    string projFilename = "../../Projekte/j60/j6014_7.sm";
	//string projFilename = "../../Projekte/j30/j301_1.sm";
    //string projFilename = "QBWLBeispiel.DAT";
    ProjectWithOvertime p(projFilename);

    GAParameters params;
    params.popSize = 80;
    params.timeLimit = -1.0;
    params.numGens = static_cast<int>(std::floor(static_cast<float>(iterLimit) / static_cast<float>(params.popSize)));
    params.fitnessBasedPairing = true;
    params.pmutate = 5;
    params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;

    auto res = GARunners::run(p, params, gaIndex);
}

int main(int argc, char * argv[]) {
	commandLineRunner(argc, argv);
	//testFixedDeadlineHeuristic();
	//testLocalSolverNative(argc == 2 ? atoi(argv[1]) : 0);
    //benchmarkGeneticAlgorithm(0, 32000);
    return 0;
}
