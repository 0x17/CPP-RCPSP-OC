#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "ProjectWithOvertime.h"
#include "LSModels/NaiveModels.h"
#include "LSModels/FixedDeadlineModels.h"
#include "Runners.h"
#include "BranchAndBound.h"
#include "GurobiSolver.h"
#include "Utils.h"

using namespace std;

namespace Main {
	void showUsage();
	void commandLineRunner(int argc, const char * argv[]);
	int computeMinMaxMakespanDifference(ProjectWithOvertime &p);
	void convertArgFileToLSP(int argc, const char * argv[]);
	bool instanceAlreadySolvedInResultFile(const string& coreName, const string& resultFilename);
	void purgeOldTraceFile(const string &traceFilename);
	void plotHeuristicProfits();
	void jsonConverter(int argc, const char** argv);

	namespace Testing {
        void fixedScheduleLimitSolveTimesForProjects();
		boost::optional<Runners::GAResult> benchmarkGeneticAlgorithm(int gaIndex, int iterLimit, const string &projFilename = "../../Projekte/j30/j3013_8.sm");
        double averageSolvetime(int gaIndex, int scheduleLimit, const string& path);
		void testFixedDeadlineHeuristic();
		void testLocalSolverNative(int seed);
		void testGurobi();
		void testSerialOptimalSubschedules();
	}
}

void Main::plotHeuristicProfits() {
	boost::filesystem::path path("../../Projekte/j30/");

	int maxDiff = numeric_limits<int>::lowest();
	string maxDiffInstance;

	for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {})) {
		ProjectWithOvertime p(entry.path().string());
		int lb = p.makespan(p.serialSGS(p.topOrder, p.zmax));
		int ub = p.makespan(p.serialSGS(p.topOrder, p.zzero));
		int diff = ub - lb;
		if(diff > maxDiff) {
			maxDiff = diff;
			maxDiffInstance = entry.path().string();
		}
	}

	ProjectWithOvertime p(maxDiffInstance);
	auto profitAndActualMakespanForDeadline = p.heuristicProfitsAndActualMakespanForRelevantDeadlines(p.topOrder);
	Utils::spit("deadline;actualMakespan;profit\n", "heurprofits.txt");
	for(auto pair : profitAndActualMakespanForDeadline) {
		Utils::spitAppend(to_string(pair.first) + ";" + to_string(pair.second.first) + ";" + to_string(pair.second.second) + "\n", "heurprofits.txt");
	}
	/*auto gsresult = p.goldenSectionSearchBasedOptimization(p.topOrder);
	cout << p.calcProfit(gsresult) << endl;*/
}

void Main::jsonConverter(int argc, const char** argv) {
	const auto showConverterUsage = []() {
		cout << "Usage: Converter inpath outpath" << endl;
		cout << "Example usage: Converter ../../Projekte/j30/j3010_1.sm j3010_1.json" << endl;
	};

	vector<string> args = Utils::parseArgumentList(argc, argv);

	if (args.empty()) {
		showConverterUsage();
		return;
	}

	const string inpath = !args.empty() ? args[0] : "../../Projekte/j30/j3010_1.sm";
	const auto projectName = boost::filesystem::path(inpath).stem().string();
	const string outpath = args.size() > 1 ? args[1] : projectName + ".json";

	ProjectWithOvertime p(inpath);
	p.to_disk(outpath);
}

void computeScheduleAttributes(const string &smfilename) {
	vector<string> resfiles = {"result_sts_0.txt", "result_sts_1.txt", "result_sts_2.txt", "result_sts_oc.txt"};
	ProjectWithOvertime p(smfilename);
	for(const auto &resfile : resfiles) {
		vector<int> sts = Utils::deserializeSchedule(p.numJobs, resfile);
		cout << resfile << " profit=" << p.calcProfit(sts) << ", makespan=" << p.makespan(sts) << endl;
	}
}

int main(int argc, const char * argv[]) {
	//computeScheduleAttributes("PaperBeispiel.sm");

	//Main::plotHeuristicProfits();

	//Main::Testing::testGurobi();

	//Utils::partitionDirectory("j120", 60);

    //string projFilename = "../../Projekte/j30/j3013_8.sm";
    //Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_BETA, 100000);
	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_ZR, 100000);
    //Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_ZRT, 100000);
	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_LAMBDA_GS, 100000);
	//Main::Testing::fixedScheduleLimitSolveTimesForProjects();

	//Main::jsonConverter(argc, argv);

	Main::commandLineRunner(argc, argv);

	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_RANDKEY_ZRT, 10000, "j30/j3010_1.sm");

	//Main::Testing::testGurobi();
	//Main::Testing::testSerialOptimalSubschedules();
	
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
    for(const auto &fn : filenames) {
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
	for (int i = 0; i < 11; i++) solMethods.push_back("GA" + to_string(i) + " // " + Runners::getDescription(i));
	for (int i = 0; i < 10; i++) solMethods.push_back("LocalSolverNative" + to_string(i) + " // " + Runners::getDescription(i));
	cout << "Number of arguments must be >= 4" << endl;
	cout << "Usage: Solver SolutionMethod TimeLimitInSecs ScheduleLimit ProjectFileSM [traceobj]" << endl;
	cout << "Solution methods: " << endl;
	for (const auto &method : solMethods) cout << "\t" << method << endl;
}

int Main::computeMinMaxMakespanDifference(ProjectWithOvertime &p) {
	int maxMs = p.makespan(p.serialSGS(p.topOrder));
	int minMs = p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
	return maxMs - minMs;
}

bool Main::instanceAlreadySolvedInResultFile(const string& coreName, const string& resultFilename) {
	if (!boost::filesystem::is_regular_file(resultFilename)) {
		return false;
	}

	for (const string &line : Utils::readLines(resultFilename)) {
		if (boost::starts_with(line, coreName)) {
			LOG_W("Instance " + coreName + " is already solved in result file " + resultFilename +
				  ", skipping result recomputation!");
			return true;
		}
	}

	return false;
}

void Main::purgeOldTraceFile(const string &traceFilename) {
	if (boost::filesystem::is_regular_file(traceFilename)) {
		LOG_W("Found old trace file " + traceFilename + ", deleting it!");
		boost::filesystem::remove(traceFilename);
	}
}

void Main::commandLineRunner(int argc, const char * argv[]) {
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

	    const bool traceobj = (argc == 6 && string(argv[5]) == "traceobj");

		const string parentPath = boost::filesystem::path(string(argv[4])).parent_path().string();
		const string outPath = parentPath + "_" + (iterLimit == -1 ? to_string(int(round(timeLimit))) + "secs/" : to_string(iterLimit) + "schedules/");
		string outFn = outPath;
		boost::filesystem::create_directory(boost::filesystem::path(outPath));
		const string coreName = Project::coreInstanceName(parentPath, string(argv[4]));

		srand(23);

		if(solMethod == "BranchAndBound") {
            BranchAndBound b(p, timeLimit, iterLimit);
			outFn += "BranchAndBoundResults.txt";
			if(instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(BranchAndBound::getTraceFilename(outPath, p.instanceName));
            sts = b.solve(false, traceobj, outPath);			
        } else if(boost::starts_with(solMethod, "GA")) {
			int gaIndex = stoi(solMethod.substr(2, solMethod.length()-2));
			int variant = (gaIndex == 0 && solMethod.length() == 4) ? stoi(solMethod.substr(3, 1)) : 0;
			outFn += "GA" + to_string(gaIndex) + "Results.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			//purgeOldTraceFile(traceFilenameForGeneticAlgorithm(outPath, ))
			sts = Runners::runGeneticAlgorithmWithIndex(p, { gaIndex, variant, timeLimit, iterLimit, traceobj, outPath });            
		}
		else if (solMethod == "LocalSolver") {
			outFn += "LocalSolverResults.txt";
			sts = LSSolver::solve(p, timeLimit, iterLimit, traceobj, outPath);			
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
			int lsnIndex = stoi(solMethod.substr(17, 1));
			int variant = (lsnIndex == 0 && solMethod.length() == 19) ? stoi(solMethod.substr(18, 1)) : 0;
			outFn += "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(ListModel::traceFilenameForListModel(outPath, lsnIndex, p.instanceName));
			sts = Runners::runLocalSolverModelWithIndex(p, { lsnIndex, variant, timeLimit, iterLimit, traceobj, outPath });			
        }  else if(solMethod == "Gurobi") {
#ifndef DISABLE_GUROBI
			GurobiSolver::Options opts;
			opts.outPath = outPath;
			opts.timeLimit = (timeLimit == -1.0) ? opts.timeLimit : timeLimit;
			opts.iterLimit = (iterLimit == -1.0) ? opts.iterLimit : iterLimit;
			opts.threadCount = 1;
			GurobiSolver gsolver(p, opts);
			gsolver.buildModel();
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
        
		// FIXME: specify number of decimal places!
        string resStr = (sts[0] == Project::UNSCHEDULED) ? "infes" : to_string(p.calcProfit(sts));
        Utils::spitAppend(coreName+";"+resStr+"\n", outFn);

		Utils::serializeSchedule(sts, outPath + "myschedule.txt");
		Utils::serializeProfit(p.calcProfit(sts), outPath + "myprofit.txt");
	}
	else showUsage();
}

#ifndef DISABLE_GUROBI
void Main::Testing::testGurobi() {
	string projFilename = "j30/j3027_4.sm";
	ProjectWithOvertime p(projFilename);
	GurobiSolver::Options opts;
	opts.traceobj = false;
	opts.useSeedSol = true;
	opts.displayInterval = 1;
	opts.gap = 0.0;
	opts.outPath = "SOMEPREFIX";
	opts.timeLimit = 60; //GRB_INFINITY;
	opts.threadCount = 0;
	GurobiSolver solver(p, opts);
	solver.buildModel();
	auto res = solver.solve();
	cout << "Profit = " << p.calcProfit(res.sts) << endl;
	Utils::serializeSchedule(res.sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res.sts), "myprofit.txt");
}

void Main::Testing::testSerialOptimalSubschedules() {
	string projFilename = "j30/j3027_4.sm";
	ProjectWithOvertime p(projFilename);
	const auto resopt = p.serialOptimalSubSGS(p.topOrder, 32);

	vector<int> order = p.scheduleToActivityList(resopt);
	//order = p.topOrder;

	const auto res = p.serialOptimalSubSGS(order, 4);
	cout << "Profit = " << p.calcProfit(res) << endl;
	Utils::serializeSchedule(res, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res), "myprofit.txt");
	getchar();
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
	string projFilename = "Data/MiniBeispiel.DAT";
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

