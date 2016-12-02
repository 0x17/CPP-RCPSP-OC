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
	void convertArgFileToLSP(int argc, const char * argv[]);
	bool instanceAlreadySolvedInResultFile(const string& coreName, const string& resultFilename);
	void purgeOldTraceFile(const string &traceFilename);

	namespace Testing {
        void fixedScheduleLimitSolveTimesForProjects();
		boost::optional<Runners::GAResult> benchmarkGeneticAlgorithm(int gaIndex, int iterLimit, const string &projFilename = "../../Projekte/j30/j3013_8.sm");
        double averageSolvetime(int gaIndex, int scheduleLimit, const string& path);
		void testFixedDeadlineHeuristic();
		void testLocalSolverNative(int seed);
		void testGurobi();
	};
}

int main(int argc, char * argv[]) {
	Main::commandLineRunner(argc, argv);

    //string projFilename = "../../Projekte/j30/j3013_8.sm";
    //Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_BETA, 100000);
	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_ZR, 100000);
    //Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_ZRT, 100000);
	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_GS, 100000);
    //Main::Testing::fixedScheduleLimitSolveTimesForProjects();
    return 0;
}

void Main::Testing::fixedScheduleLimitSolveTimesForProjects() {
    string j30path = "/Users/andreschnabel/Dropbox/Arbeit/Scheduling/Projekte/j30/";
    vector<int> relevantGaIndices = {Runners::RE_LAMBDA_BETA, Runners::RE_LAMBDA_ZR, /*Runners::RE_LAMBDA_ZRT,*/ Runners::RE_LAMBDA_GS };
    vector<double> avgSolvetimes(relevantGaIndices.size());
    int ctr =0;
    for(int ix : relevantGaIndices) {
        avgSolvetimes[ctr++] = Main::Testing::averageSolvetime(ix, 1000, j30path);
    }
    for(int i=0; i<avgSolvetimes.size(); i++)
        cout << "Method " << Runners::getDescription(relevantGaIndices[i]) << " average solvetime = " << avgSolvetimes[i] << endl;
}

double Main::Testing::averageSolvetime(int gaIndex, int scheduleLimit, const string &path) {
    double result = 0.0;
    int ctr = 0;
    auto filenames = Utils::filenamesInDirWithExt(path, ".sm");
    for(auto fn : filenames) {
        auto res = Main::Testing::benchmarkGeneticAlgorithm(gaIndex, scheduleLimit, fn);
        if(res) {
            result += res.get().solvetime;
            ctr++;
        }
    }
    return result / static_cast<double>(ctr);
}

void Main::convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

void Main::showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver", "Gurobi" };
	for (int i = 0; i < 8; i++) solMethods.push_back("GA" + to_string(i) + " // " + Runners::getDescription(i));
	for (int i = 0; i < 8; i++) solMethods.push_back("LocalSolverNative" + to_string(i) + " // " + Runners::getDescription(i));
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

bool Main::instanceAlreadySolvedInResultFile(const string& coreName, const string& resultFilename) {
	for(string line : Utils::readLines(resultFilename)) {
		if (boost::starts_with(line, coreName)) {
			LOG_W("Instance " + coreName + " is already solved in result file " + resultFilename + ", skipping result recomputation!");
			return true;
		}
	}
	return false;
}

void Main::purgeOldTraceFile(const string &traceFilename) {
	if (boost::filesystem::exists(traceFilename)) {
		LOG_W("Found old trace file " + traceFilename + ", deleting it!");
		boost::filesystem::remove(traceFilename);
	}
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
		string coreName = Project::coreInstanceName(parentPath, string(argv[4]));

		if(!solMethod.compare("BranchAndBound")) {
            BranchAndBound b(p, timeLimit, iterLimit);
			outFn += "BranchAndBoundResults.txt";
			if(instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(BranchAndBound::getTraceFilename(outPath, p.instanceName));
            sts = b.solve(false, traceobj, outPath);			
        } else if(boost::starts_with(solMethod, "GA")) {
			int gaIndex = stoi(solMethod.substr(2, 1));
			int variant = (gaIndex == 0 && solMethod.length() == 4) ? stoi(solMethod.substr(3, 1)) : 0;
			outFn += "GA" + to_string(gaIndex) + "Results.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			//purgeOldTraceFile(traceFilenameForGeneticAlgorithm(outPath, ))
			sts = Runners::runGeneticAlgorithmWithIndex(p, { gaIndex, variant, timeLimit, iterLimit, traceobj, outPath });            
		}
		else if (!solMethod.compare("LocalSolver")) {
			outFn += "LocalSolverResults.txt";
			sts = LSSolver::solve(p, timeLimit, iterLimit, traceobj, outPath);			
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
			int lsnIndex = stoi(solMethod.substr(17, 1));
			int variant = (lsnIndex == 0 && solMethod.length() == 19) ? stoi(solMethod.substr(18, 1)) : 0;
			outFn += "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(ListModel::traceFilenameForListModel(outPath, lsnIndex, p.instanceName));
			sts = Runners::runLocalSolverModelWithIndex(p, { lsnIndex, variant, timeLimit, iterLimit, traceobj, outPath });			
        }  else if(!solMethod.compare("Gurobi")) {
#ifndef DISABLE_GUROBI
			GurobiSolver::Options opts;
			opts.outPath = outPath;
			opts.timeLimit = (timeLimit == -1.0) ? opts.timeLimit : timeLimit;
			opts.iterLimit = (iterLimit == -1.0) ? opts.iterLimit : iterLimit;
			opts.threadCount = 4;
			GurobiSolver gsolver(p, opts);
			outFn += "GurobiResults.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(GurobiSolver::traceFilenameForInstance(outPath, p.instanceName));
			auto res = gsolver.solve();
			if(res.optimal) {
				Utils::spitAppend(coreName + "\n", "GurobiOptimals.txt");
			}
			sts = res.sts;			
#endif
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

#ifndef DISABLE_GUROBI
void Main::Testing::testGurobi() {
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
#endif

void Main::Testing::testFixedDeadlineHeuristic() {
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

void Main::Testing::testLocalSolverNative(int seed) {
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

boost::optional<Runners::GAResult> Main::Testing::benchmarkGeneticAlgorithm(int gaIndex, int iterLimit, const string &projFilename) {
    ProjectWithOvertime p(projFilename);

    if(computeMinMaxMakespanDifference(p) <= 0) {
        cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
        return boost::optional<Runners::GAResult>();
    }

    GAParameters params;
    params.popSize = 80;
    params.timeLimit = -1;
    params.numGens = -1; //static_cast<int>(floor(static_cast<float>(iterLimit) / static_cast<float>(params.popSize)));
    params.fitnessBasedPairing = false;
    params.pmutate = 5;
    params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;
    params.iterLimit = iterLimit;

    auto res = Runners::run(p, params, gaIndex);
	//bool feas = p.isScheduleFeasible(res.sts);
	//cout << feas << endl;

	Utils::serializeSchedule(res.sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res.sts), "myprofit.txt");
	//cout << (p.calcProfit(res.sts) == res.profit) << endl;

	cout << "Solvetime = " << res.solvetime << endl;

    return res;
}

