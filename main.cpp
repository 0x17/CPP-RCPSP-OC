#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "ProjectWithOvertime.h"
#include "LSModels/NaiveModels.h"
#include "LSModels/FixedDeadlineModels.h"
#include "Runners.h"
#include "BranchAndBound.h"
#include "GurobiSolver.h"

namespace Main {
	void showUsage();
	void commandLineRunner(int argc, char * argv[]);

	int computeMinMaxMakespanDifference(ProjectWithOvertime &p);

	void benchmarkGeneticAlgorithm(int gaIndex, int iterLimit);

	void testFixedDeadlineHeuristic();
	void testLocalSolverNative(int seed);
	void testGurobi();

	void convertArgFileToLSP(int argc, const char * argv[]);
}

int main(int argc, char * argv[]) {
	Main::commandLineRunner(argc, argv);
	return 0;
}

void Main::convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

void Main::testGurobi() {
	string projFilename = "QBWLBeispiel.DAT";
	ProjectWithOvertime p(projFilename);
	GurobiSolver::Options opts;
	opts.useSeedSol = true;
	opts.displayInterval = 1;
	opts.gap = 0.0;
	opts.outPath = "";
	opts.timeLimit = GRB_INFINITY;
	opts.threadCount = 0;
	GurobiSolver solver(p, opts);
	auto res = solver.solve();
}

void Main::showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver", "Gurobi" };
	for (int i = 0; i < 8; i++) solMethods.push_back("GA" + to_string(i) + " // " + Runners::getGADescription(i));
	for (int i = 0; i < 8; i++) solMethods.push_back("LocalSolverNative" + to_string(i) + " // " + Runners::getLSDescription(i));
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

string coreInstanceName(const string & parentPath, const string & filename) {
	string fn(filename);
	boost::replace_first(fn, parentPath + "/", "");
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

		string parentPath = boost::filesystem::path(string(argv[4])).parent_path().string();
		string outPath = parentPath + "_" + to_string(int(round(timeLimit))) + "secs/";
		string outFn = outPath;
		boost::filesystem::create_directory(boost::filesystem::path(outPath));
		string coreName = coreInstanceName(parentPath, string(argv[4]));

		if(!solMethod.compare("BranchAndBound")) {
            BranchAndBound b(p, timeLimit, iterLimit);
            sts = b.solve(false, traceobj, outPath);
            outFn += "BranchAndBoundResults.txt";
        } else if(boost::starts_with(solMethod, "GA")) {
			int gaIndex = stoi(solMethod.substr(2, 1));
			int variant = (gaIndex == 0 && solMethod.length() == 4) ? stoi(solMethod.substr(3, 1)) : 0;
			sts = Runners::runGeneticAlgorithmWithIndex(p, { gaIndex, variant, timeLimit, iterLimit, traceobj, outPath });
            outFn += "GA" + to_string(gaIndex) + "Results.txt";
		}
		else if (!solMethod.compare("LocalSolver")) {
			sts = LSSolver::solve(p, timeLimit, iterLimit, traceobj, outPath);
			outFn += "LocalSolverResults.txt";
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
			int lsnIndex = stoi(solMethod.substr(17, 1));
			int variant = (lsnIndex == 0 && solMethod.length() == 19) ? stoi(solMethod.substr(18, 1)) : 0;
			sts = Runners::runLocalSolverModelWithIndex(p, { lsnIndex, variant, timeLimit, iterLimit, traceobj, outPath });
			outFn += "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
        }  else if(!solMethod.compare("Gurobi")) {
			GurobiSolver::Options opts;
			opts.outPath = outPath;
			opts.timeLimit = (timeLimit == -1.0) ? opts.timeLimit : timeLimit;
			opts.iterLimit = (iterLimit == -1.0) ? opts.iterLimit : iterLimit;
			opts.threadCount = 4;
			GurobiSolver gsolver(p, opts);
			auto res = gsolver.solve();
			if(res.optimal) {
				Utils::spitAppend(coreName + "\n", "GurobiOptimals.txt");
			}
			sts = res.sts;
			outFn += "GurobiResults.txt";
        } else {
			throw runtime_error("Unknown method: " + solMethod + "!");
        }
        
        string resStr = (sts[0] == Project::UNSCHEDULED) ? "infes" : to_string(p.calcProfit(sts));
        Utils::spitAppend(coreName+";"+resStr+"\n", outFn);

		Utils::serializeSchedule(sts, outPath + "myschedule.txt");
		Utils::serializeProfit(p.calcProfit(sts), outPath + "myprofit.txt");
	}
	else showUsage();
}

void Main::testFixedDeadlineHeuristic() {
	string projFilename = "../../Projekte/j30/j301_3.sm";
	ProjectWithOvertime p(projFilename);

	GAParameters params;
	params.popSize = 80;
	params.timeLimit = -1.0;
	params.numGens = 100;
	params.fitnessBasedPairing = true;
	params.pmutate = 5;
	params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;

	auto res = Runners::run(p, params, 6);
	auto sts = res.sts;

	p.isScheduleFeasible(sts);

	//Utils::serializeSchedule(sts, "myschedule.txt");
	//Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedule.txt");
}

void Main::testLocalSolverNative(int seed) {
	string projFilename = "MiniBeispiel.DAT";
	ProjectWithOvertime p(projFilename);
	//ListAlternativesModel lm(p);
	ListDeadlineModel lm(p);
	SolverParams params(5.0);
	params.traceobj = true;
	params.seed = seed;
	//params.solverIx = 8;
	auto sts = lm.solve(params);
	Utils::serializeSchedule(sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe ../../Projekte/j30/j301_1.sm myschedule.txt");
}

void Main::benchmarkGeneticAlgorithm(int gaIndex, int iterLimit) {
	string projFilename = "../../Projekte/j30/j3013_8.sm";
    ProjectWithOvertime p(projFilename);

    GAParameters params;
    params.popSize = 80;
    params.timeLimit = 5;
    params.numGens = -1; //static_cast<int>(floor(static_cast<float>(iterLimit) / static_cast<float>(params.popSize)));
    params.fitnessBasedPairing = false;
    params.pmutate = 5;
    params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;

    auto res = Runners::run(p, params, gaIndex);
	bool feas = p.isScheduleFeasible(res.sts);
	cout << feas << endl;

	Utils::serializeSchedule(res.sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res.sts), "myprofit.txt");
	cout << (p.calcProfit(res.sts) == res.profit) << endl;
}

