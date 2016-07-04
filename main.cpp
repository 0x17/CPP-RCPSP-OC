#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "ProjectWithOvertime.h"

#include "LSModels/NaiveModels.h"
#include "LSModels/OvertimeBoundModels.h"
#include "LSModels/TimeWindowModels.h"
#include "LSModels/FixedDeadlineModels.h"

#include "GeneticAlgorithms/Runners.h"
#include "GeneticAlgorithms/TimeWindow.h"

#include "BranchAndBound.h"

namespace Main {
	void showUsage();
	void commandLineRunner(int argc, char * argv[]);

	int computeMinMaxMakespanDifference(ProjectWithOvertime &p);

	ListModel *genListModelWithIndex(ProjectWithOvertime &p, int index, int variant = -1);
	vector<int> runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, int gaIndex, int variant, double timeLimit, int iterLimit, bool traceobj, string outPath);
	vector<int> runLocalSolverModelWithIndex(ProjectWithOvertime &p, int lsIndex, int variant, double timeLimit, int iterLimit, bool traceobj, string outPath);

	void benchmarkGeneticAlgorithm(int gaIndex, int iterLimit);

	void testFixedDeadlineHeuristic();
	void testLocalSolverNative(int seed);

	void convertArgFileToLSP(int argc, const char * argv[]);
}

int main(int argc, char * argv[]) {
	Main::commandLineRunner(argc, argv);
	//Main::testFixedDeadlineHeuristic();
	//Main::testLocalSolverNative(argc == 2 ? atoi(argv[1]) : 0);
	//Main::benchmarkGeneticAlgorithm(5, 256000);
	return 0;
}

void Main::convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

void Main::showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver" };
	for (int i = 0; i < 7; i++) solMethods.push_back("GA" + to_string(i));
	for (int i = 0; i < 9; i++) solMethods.push_back("LocalSolverNative" + to_string(i));
	cout << "Number of arguments must be >= 4" << endl;
	cout << "Usage: Solver SolutionMethod TimeLimitInSecs ScheduleLimit ProjectFileSM [traceobj]" << endl;
	cout << "Solution methods: " << endl;
	for (auto method : solMethods) cout << "\t" << method << endl;
}

int Main::computeMinMaxMakespanDifference(ProjectWithOvertime &p) {
	int maxMs = p.makespan(p.serialSGS(p.topOrder));
	int minMs = p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
	return maxMs - minMs;
}

ListModel *Main::genListModelWithIndex(ProjectWithOvertime &p, int index, int variant) {
	ListModel *lm = nullptr;
	switch (index) {
	default:
	case 0:
		ListBetaModel::setVariant(variant);
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
	case 6:
		lm = new ListDeadlineModel(p);
		break;
	case 7:
		lm = new ListBetaDeadlineModel(p);
		break;
	case 8:
		lm = new ListTauDeadlineModel(p);
		break;
	}
	return lm;
}

vector<int> Main::runGeneticAlgorithmWithIndex(ProjectWithOvertime &p, int gaIndex, int variant, double timeLimit, int iterLimit, bool traceobj, string outPath) {
	GAParameters params;
	params.fitnessBasedPairing = true;
	params.numGens = -1;
	params.popSize = 80;
	params.pmutate = 5;
	params.timeLimit = timeLimit;
	params.iterLimit = iterLimit;
	params.traceobj = traceobj;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;
	params.outPath = outPath;

	if(gaIndex == 0)
		TimeWindowBordersGA::setVariant(variant == -1 ? 0 : variant);

	auto res = GARunners::run(p, params, gaIndex);
	return res.sts;
}

vector<int> Main::runLocalSolverModelWithIndex(ProjectWithOvertime& p, int lsIndex, int variant, double timeLimit, int iterLimit, bool traceobj, string outPath) {
	vector<int> sts;
	ListModel *lm = genListModelWithIndex(p, lsIndex, variant);
	SolverParams params(timeLimit, iterLimit);
	params.trace = traceobj;
	params.solverIx = lsIndex;
	params.outPath = outPath;
	sts = lm->solve(params);
	delete lm;
	return sts;
}

string coreInstanceName(const string & filename) {
	string fn(filename);
	boost::replace_first(fn, "j30/", "");
	boost::replace_first(fn, ".sm", "");
	return fn;
}

void Main::commandLineRunner(int argc, char * argv[]) {
    if(argc >= 4) {
		vector<int> sts;

        string solMethod = argv[1];
        double timeLimit = atof(argv[2]);
		int iterLimit = atoi(argv[3]);
        ProjectWithOvertime p(argv[4]);
        
        if(computeMinMaxMakespanDifference(p) <= 0) {
            cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
            return;
        }

        bool traceobj = (argc == 6 && !string("traceobj").compare(argv[5]));

		string outPath = boost::filesystem::path(string(argv[4])).parent_path().string() + "_" + to_string(int(round(timeLimit))) + "secs/";
		string outFn = outPath;
		boost::filesystem::create_directory(boost::filesystem::path(outPath));

		if(!solMethod.compare("BranchAndBound")) {
            BranchAndBound b(p, timeLimit, iterLimit);
            sts = b.solve(false, traceobj, outPath);
            outFn += "BranchAndBoundResults.txt";
        } else if(boost::starts_with(solMethod, "GA")) {
			int gaIndex = stoi(solMethod.substr(2, 1));
			int variant = (gaIndex == 0 && solMethod.length() == 4) ? stoi(solMethod.substr(3, 1)) : -1;
	        sts = runGeneticAlgorithmWithIndex(p, gaIndex, variant, timeLimit, iterLimit, traceobj, outPath);
            outFn += "GA" + to_string(gaIndex) + "Results.txt";
		}
		else if (!solMethod.compare("LocalSolver")) {
			sts = LSSolver::solve(p, timeLimit, iterLimit, traceobj, outPath);
			outFn += "LocalSolverResults.txt";
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
			int lsnIndex = stoi(solMethod.substr(17, 1));
			int variant = (lsnIndex == 0 && solMethod.length() == 19) ? stoi(solMethod.substr(18, 1)) : -1;
			sts = runLocalSolverModelWithIndex(p, lsnIndex, variant, timeLimit, iterLimit, traceobj, outPath);
			outFn += "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
        } else {
			throw runtime_error("Unknown method: " + solMethod + "!");
        }
        
        string resStr = (sts[0] == Project::UNSCHEDULED) ? "infes" : to_string(p.calcProfit(sts));
        Utils::spitAppend(coreInstanceName(string(argv[4]))+";"+resStr+"\n", outFn);

		Utils::serializeSchedule(sts, outPath + "myschedule.txt");
		Utils::serializeProfit(p.calcProfit(sts), outPath + "myprofit.txt");
	}
	else showUsage();
}

void Main::testFixedDeadlineHeuristic() {
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

void Main::testLocalSolverNative(int seed) {
	//string projFilename = "../../Projekte/j30/j3010_1.sm";
	//string projFilename = "../../Projekte/j30/j301_1.sm";
	//string projFilename = "QBWLBeispiel.DAT";
	string projFilename = "MiniBeispiel.DAT";
	ProjectWithOvertime p(projFilename);
	//ListAlternativesModel lm(p);
	ListBetaDeadlineModel lm(p);
	SolverParams params(5.0);
	params.trace = true;
	params.seed = seed;
	//params.solverIx = 8;
	auto sts = lm.solve(params);
	Utils::serializeSchedule(sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe ../../Projekte/j30/j301_1.sm myschedule.txt");
}

void Main::benchmarkGeneticAlgorithm(int gaIndex, int iterLimit) {
    //string projFilename = "../../Projekte/j60/j6014_7.sm";
	//string projFilename = "../../Projekte/j30/j301_1.sm";
    string projFilename = "QBWLBeispiel.DAT";
	//string projFilename = "MiniBeispiel.DAT";
	//string projFilename = "../../Projekte/j30/j3010_1.sm";
	//string projFilename = "../../Projekte/j30/j301_1.sm";
    ProjectWithOvertime p(projFilename);

    GAParameters params;
    params.popSize = 80;
    params.timeLimit = -1.0;
    params.numGens = static_cast<int>(floor(static_cast<float>(iterLimit) / static_cast<float>(params.popSize)));
    params.fitnessBasedPairing = true;
    params.pmutate = 5;
    params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;

    auto res = GARunners::run(p, params, gaIndex);
}


