#include <cmath>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>

#include "ProjectWithOvertime.h"

#include "Runners.h"
#include "BranchAndBound.h"
#include "GurobiSolver.h"
#include "Utils.h"

#include "SensitivityAnalysis.h"

#include "GeneticAlgorithms/Partition.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/TimeWindow.h"

#include "LSModels/NaiveModels.h"
#include "LSModels/FixedDeadlineModels.h"
#include "LSModels/PartitionModels.h"
#include "LSModels/SimpleModel.h"

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
	void charactersticCollector(int argc, const char **argv);

	namespace Testing {
		void fixedScheduleLimitSolveTimesForProjects();
		boost::optional<Runners::GAResult> benchmarkGeneticAlgorithm(int gaIndex, int iterLimit, const string &projFilename = "../../Projekte/j30/j3013_8.sm");
		double averageSolvetime(int gaIndex, int scheduleLimit, const string& path);
		void testFixedDeadlineHeuristic();
		void testLocalSolverNative(int seed);
		void testGurobi();
		void testSerialOptimalSubschedules();
		void testLocalSolverNativePartitionList();
		void testLocalSolverNativePartitions();
		void testOptimalSubschedulesGA();
		void testPartitionListSGS();
		void testRandomKeyGAZrt();
		void testParallelSGSGAZrt();
		void testLocalSolverForRCPSP();
		void testLocalSolverForRCPSPROC();
		void testCollectCharacteristics();

		struct GAConfigurationExperiment;
		void tweakGAParameters(const string &projFilename, int iterLimit, GAConfigurationExperiment &experiment);
	}
}

struct Main::Testing::GAConfigurationExperiment {
	string parameterName;
	int lb, ub, step;
};

int main(int argc, const char * argv[]) {
	//ProjectWithOvertime p("j30/j301_1.sm");
	//cout << "haha" << endl;

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

	//Main::charactersticCollector(argc, argv);

	Main::commandLineRunner(argc, argv);

	//sensitivity::varyTotalAvailableCapacity("j30/j3029_9.sm", 0);

	//Main::Testing::GAConfigurationExperiment varyMutationProb = { "pmutate", 0, 25, 1 };
	//Main::Testing::tweakGAParameters("PaperBeispiel.sm", 100, varyMutationProb);

	//Main::Testing::testCollectCharacteristics();

	//Main::Testing::testLocalSolverForRCPSP();
	//Main::Testing::testLocalSolverForRCPSPROC();

	//Main::Testing::testPartitionListSGS();

	//Main::Testing::testRandomKeyGAZrt();
	//Main::Testing::testParallelSGSGAZrt();

	//Main::Testing::testLocalSolverNativePartitionList();

	//GAParameters params;
	//Utils::spit(params.toJson().dump(), "GAParameters.json");

	//Main::Testing::benchmarkGeneticAlgorithm(Runners::RepresentationEnum::RE_RANDKEY_ZRT, 10000, "j30/j3010_1.sm");

	//Main::Testing::testGurobi();

	//Main::Testing::testSerialOptimalSubschedules();

	//Main::Testing::testOptimalSubschedulesGA();

	return 0;
}

void Main::Testing::testPartitionListSGS() {
	const ProjectWithOvertime p("PartitionBeispiel.sm");
	const vector<int> plist = { 0, 0, 0, 0, 1, 1, 1, 1 };
	const vector<int> sts = p.serialOptimalSubSGSWithPartitionList(plist);
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

template<class Func>
void applyForAllProjectsInDirectory(Func f, const string &pathToDirectory, const vector<string> &projectExtensions, bool skipEqualMakespans = true) {
	list<string> instanceFilenames;

	for(const string &projectExtension : projectExtensions) {
		const auto filenamesWithExtension = Utils::filenamesInDirWithExt(pathToDirectory, projectExtension);
		for(const string &filename : filenamesWithExtension) {
			instanceFilenames.push_back(filename);
		}
	}

	const int len = instanceFilenames.size();
	int ctr = 1;
	for(const auto &instancefn : instanceFilenames) {
		ProjectWithOvertime p(instancefn);
		if(skipEqualMakespans && Main::computeMinMaxMakespanDifference(p) <= 0) {
			cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
			continue;
		}
		f(p, ctr++, len);
	}
}

std::map<std::string, float> quickGAResults(ProjectWithOvertime &p) {
	GAParameters params;
	params.traceobj = false;
	params.numGens = -1;
	params.iterLimit = 1000;
	params.timeLimit = -1;

	vector<pair<vector<int>, float>> results;

	cout << "Instance = " << p.instanceName << endl;

	{
		TimeWindowBordersGA ga(p);
		ga.setParameters(params);
		results.push_back(ga.solve());
	}

	{
		FixedCapacityGA ga(p);
		ga.setParameters(params);
		results.push_back(ga.solve());
	}

	{
		TimeVaryingCapacityGA ga(p);
		ga.setParameters(params);
		results.push_back(ga.solve());
	}


	return map<string, float> {
		{"qresga0obj", results[0].second },
		{"qresga3obj", results[1].second },
		{"qresga4obj", results[2].second },
		{"qresga0ms", p.makespan(results[0].first) },
		{"qresga3ms", p.makespan(results[1].first) },
		{"qresga4ms", p.makespan(results[2].first) },
		{"qresga0cost", p.totalCosts(results[0].first) },
		{"qresga3cost", p.totalCosts(results[1].first) },
		{"qresga4cost", p.totalCosts(results[2].first) },
	};
}

static string limitDecimals(float n) {
	stringstream ss;
	ss << std::setprecision(3) << n;
	return ss.str();
};

string joinFloats(const vector<float> &v, const string &sep) {
	const vector<string> strs = Utils::constructVector<string>(v.size(), [&v](int ix) {
		return limitDecimals(v[ix]);
	});
	return boost::algorithm::join(strs, sep);
}

void Main::charactersticCollector(int argc, const char** argv) {
	const vector<string> instanceFileExtensions = {".sm", ".rcp"};

	const auto showCollectorUsage = []() {
		cout << "Usage: Collector [instancePath] [outfile] [noheader]" << endl;
		cout << "Example usage: Collector j30 characteristics.txt noheader" << endl;
	};

	vector<string> args = Utils::parseArgumentList(argc, argv);

	if (args.empty()) {
		showCollectorUsage();
		return;
	}

	const string instancePath = !args.empty() ? args[0] : "j30";
	const string outfn = args.size() >= 2 ? args[1] : "characteristics.txt";
	const string outfnFlattened = args.size() >= 3 ? args[2] : "flattened.txt";
	const bool noHeader = args.size() >= 4 && args[3] == "noheader";

	string ostr, ostrFlattened;

	applyForAllProjectsInDirectory([&ostr, &ostrFlattened, noHeader](const ProjectWithOvertime &p, int ctr, int len) {
		cout << "\rInstance " << p.instanceName << ", progress " << ((float)ctr/(float)len*100.0f);
		fflush(stdout);

		const auto messelisStats = p.collectMesselisStats();

		const auto characteristics = p.collectCharacteristics(/*quickGAResults(p)*/messelisStats);

		if(ostr.empty() && !noHeader) {
			ostr += characteristics.csvHeaderLine();;
		}
		ostr += characteristics.toCsvLine();

		const auto flattenedValues = p.flattenedRepresentation(characteristics);

		if(ostrFlattened.empty()) {
			ostrFlattened += "instance;" + boost::algorithm::join(flattenedValues.first, ";") + "\n";
		}

		ostrFlattened += p.instanceName + ";" + joinFloats(flattenedValues.second, ";") + "\n";

	}, instancePath, instanceFileExtensions, false);

	Utils::spit(ostr, outfn);
	Utils::spit(ostrFlattened, outfnFlattened);
}

void computeScheduleAttributes(const string &smfilename) {
	vector<string> resfiles = {"result_sts_0.txt", "result_sts_1.txt", "result_sts_2.txt", "result_sts_oc.txt"};
	ProjectWithOvertime p(smfilename);
	for(const auto &resfile : resfiles) {
		vector<int> sts = Utils::deserializeSchedule(p.numJobs, resfile);
		cout << resfile << " profit=" << p.calcProfit(sts) << ", makespan=" << p.makespan(sts) << endl;
	}
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
#ifndef DISABLE_LOCALSOLVER
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
#endif
}

void Main::showUsage() {
	list<string> solMethods = { "BranchAndBound", "LocalSolver", "Gurobi" };
	for (int i = 0; i < 12; i++) solMethods.push_back("GA" + to_string(i) + " // " + Runners::getDescription(i));
	for (int i = 0; i < 11; i++) solMethods.push_back("LocalSolverNative" + to_string(i) + " // " + Runners::getDescription(i));
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

	    bool traceobj, quiet, timeforbks, info, noskip;
        map<string, bool *> lastParameterToggles = {
        		{"traceobj", &traceobj },
        		{"quiet", &quiet },
        		{"timeforbks", &timeforbks },
				{"info", &info},
				{"noskip", &noskip}
        };

		for (const auto &pair : lastParameterToggles) {
			*pair.second = false;
		}

		for(int c=5; c<argc; c++) {
			for (const auto &pair : lastParameterToggles) {
				if(string(argv[c]) == pair.first) {
					*pair.second = true;
					break;
				}
			}
		}

		if(!noskip && computeMinMaxMakespanDifference(p) <= 0) {
			cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
			return;
		}

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
#ifndef DISABLE_LOCALSOLVER
			outFn += "LocalSolverResults.txt";
			sts = LSSolver::solve(p, timeLimit, iterLimit, traceobj, outPath);
#endif
		} else if(boost::starts_with(solMethod, "LocalSolverNative")) {
#ifndef DISABLE_LOCALSOLVER
			int lsnIndex = stoi(solMethod.substr(17, solMethod.length()-17));
			int variant = (lsnIndex == 0 && solMethod.length() == 19) ? stoi(solMethod.substr(18, 1)) : 0;
			outFn += "LocalSolverNative" + to_string(lsnIndex) + "Results.txt";
			if (instanceAlreadySolvedInResultFile(coreName, outFn)) return;
			purgeOldTraceFile(LSBaseModel::traceFilenameForLSModel(outPath, lsnIndex, p.instanceName));
			sts = Runners::runLocalSolverModelWithIndex(p, { lsnIndex, variant, timeLimit, iterLimit, traceobj, outPath });
#endif
        }  else if(solMethod == "Gurobi") {
#ifndef DISABLE_GUROBI
			GurobiSolver::Options opts;
			opts.outPath = outPath;
			opts.timeLimit = (timeLimit == -1.0) ? opts.timeLimit : timeLimit;
			opts.iterLimit = (iterLimit == -1.0) ? opts.iterLimit : iterLimit;
			opts.threadCount = 1;
			opts.saveTimeForBks = timeforbks;
			auto mipStartSts = boost::optional<const vector<int> &>();
			if(opts.timeLimit == -1 && opts.iterLimit == 1) {
				LOG_I("no limits -> compute exact reference values with GA start and as much threads as quickly as possible");
				opts.threadCount = 0;
				GAParameters gaParams;
				gaParams.timeLimit = 10;
				TimeVaryingCapacityGA tvcga(p);
				tvcga.setParameters(gaParams);
				const auto gaResPair = tvcga.solve();
				mipStartSts = gaResPair.first;
				LOG_I("GA finished, now starting MIP solve with start solution with profit " + to_string(gaResPair.second));
			}
			GurobiSolver gsolver(p, opts, mipStartSts);
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

        if(info) {
        	p.printScheduleInformation(sts);
        }

        if(quiet) return;
        
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
	string projFilename = "j30/j3021_8.sm"; //"j30/j3010_1.sm";
	ProjectWithOvertime p(projFilename);
	GurobiSolver::Options opts;
	opts.traceobj = false;
	opts.useSeedSol = true;
	opts.displayInterval = 1;
	opts.gap = 0.0;
	opts.outPath = "SOMEPREFIX";
	opts.timeLimit = GRB_INFINITY;
	opts.threadCount = 0;
	GurobiSolver solver(p, opts);
	solver.buildModel();
	auto res = solver.solve();
	cout << "Profit = " << p.calcProfit(res.sts) << endl;
	Utils::serializeSchedule(res.sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res.sts), "myprofit.txt");
	system(("java -jar ScheduleValidator.jar . " + projFilename).c_str());
}

void Main::Testing::testSerialOptimalSubschedules() {
	string projFilename = "j30/j3010_1.sm";
	ProjectWithOvertime p(projFilename);
	//const auto resopt = p.serialOptimalSubSGS(p.topOrder, 32);
	//vector<int> order = p.scheduleToActivityList(resopt);
	vector<int> order = p.topOrder;

	const auto res = p.serialOptimalSubSGS(order, 8);
	cout << "Profit = " << p.calcProfit(res) << endl;
	//Utils::serializeSchedule(res, "myschedule.txt");
	//Utils::serializeProfit(p.calcProfit(res), "myprofit.txt");
	//getchar();
}

#endif

#ifndef DISABLE_LOCALSOLVER
void Main::Testing::testLocalSolverNativePartitionList() {
	ProjectWithOvertime p("j30/j3010_1.sm");
	const auto res = LSSolver::solvePartitionListModel(p);
	cout << "Profit = " << p.calcProfit(res) << endl;
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
	params.enforceTopOrdering = true;
	params.partitionSize = 4;

	auto res = Runners::run(p, params, 6);
	auto sts = res.sts;

	p.isScheduleFeasible(sts);

	//Utils::serializeSchedule(sts, "myschedule.txt");
	//Utils::serializeProfit(p.calcProfit(sts), "myprofit.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedule.txt");
}

#ifndef DISABLE_LOCALSOLVER
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
#endif

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
	params.enforceTopOrdering = true;
	params.partitionSize = 4;

    auto res = Runners::run(p, params, gaIndex);
	//bool feas = p.isScheduleFeasible(res.sts);
	//cout << feas << endl;

	Utils::serializeSchedule(res.sts, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(res.sts), "myprofit.txt");
	//cout << (p.calcProfit(res.sts) == res.profit) << endl;

	cout << "Solvetime = " << res.solvetime << endl;

    return res;
}

#ifndef DISABLE_LOCALSOLVER
void Main::Testing::testLocalSolverNativePartitions() {
	ProjectWithOvertime p("j30/j3010_1.sm");
	PartitionsModel lm(p);
	SolverParams params(5.0);
	params.traceobj = true;
	params.seed = 23;
	params.threadCount = 1;
	params.outPath = "";
	params.timeLimit = -1;
	params.iterLimit = 1000;
	auto sts = lm.solve(params);
	cout << "Profit = " << p.calcProfit(sts) << endl;
}
#endif

template<class T>
void solveAndvalidateGAResult(const ProjectWithOvertime &p, const std::string &instanceFilename, GeneticAlgorithm<T> &ga) {
	Stopwatch sw;
	sw.start();
	const auto pair = ga.solve();
	const double solvetime = sw.look();
	const Runners::GAResult result = { pair.first, pair.second, solvetime, ga.getName() };
	LOG_I("Representation=" + result.name + " Profit=" + to_string(result.profit) + " Solvetime=" + to_string(result.solvetime));
	LOG_I("Actual profit = " + to_string(p.calcProfit(pair.first)));
	assert(p.isScheduleFeasible(pair.first));
	Utils::serializeSchedule(pair.first, "myschedule.txt");
	Utils::serializeProfit(p.calcProfit(pair.first), "myprofit.txt");
	system(("java -jar ScheduleValidator.jar . " + instanceFilename).c_str());
}

void Main::Testing::testOptimalSubschedulesGA() {
	string instanceFilename = "j30/j3021_8.sm"; // "j30/j3010_1.sm";

	ProjectWithOvertime p(instanceFilename);

	GAParameters params;
	params.popSize = 20;
	params.timeLimit = 30;
	params.numGens = -1;
	params.fitnessBasedPairing = false;
	params.pmutate = 5;
	params.traceobj = false;
	params.selectionMethod = SelectionMethod::BEST;
	params.rbbrs = true;
	params.iterLimit = -1;
	params.enforceTopOrdering = true;
	params.partitionSize = 8; // 4;

	PartitionListGA ga(p);
	//TimeVaryingCapacityGA ga(p);

	ga.setParameters(params);
	solveAndvalidateGAResult(p, instanceFilename, ga);

	//Schedule infeasible: Schedule is not order feasible: 10->11 but ST(10)+d(10)=18+1=19>17=ST(11)!
}

void Main::Testing::testRandomKeyGAZrt() {
	string instanceFilename = "j30/j3021_8.sm"; // "j30/j3010_1.sm";
	ProjectWithOvertime p(instanceFilename);
	GAParameters params;
	TimeVaryingCapacityRandomKeyGA ga(p);
	ga.setParameters(params);
	solveAndvalidateGAResult(p, instanceFilename, ga);
}

void Main::Testing::testParallelSGSGAZrt() {
	string instanceFilename = "j30/j3021_8.sm"; // "j30/j3010_1.sm";
	ProjectWithOvertime p(instanceFilename);
	GAParameters params;
	params.sgs = ScheduleGenerationScheme::PARALLEL;
	TimeVaryingCapacityGA ga(p);
	ga.setParameters(params);
	solveAndvalidateGAResult(p, instanceFilename, ga);
}

#ifndef DISABLE_LOCALSOLVER
void Main::Testing::testLocalSolverForRCPSP() {
	Project p("j30/j3021_8.sm");
	const auto sts = LSSolver::solveRCPSP(p);
	printf("");
}

void Main::Testing::testLocalSolverForRCPSPROC() {
	ProjectWithOvertime p("j30/j3021_8.sm");
	const auto sts = LSSolver::solveRCPSPROC(p);
	printf("");
}
#endif

void Main::Testing::testCollectCharacteristics() {
	ProjectWithOvertime p("j30/j3017_2.sm");
	const ProjectCharacteristics characteristics = p.collectCharacteristics();
	cout << characteristics.toCsvLine();
}


void collectResultsVaryingParam(ProjectWithOvertime &p, GAParameters &params, int &param, int from, int to, int step, const string &csvFilename) {
	string ostr = "paramValue;makespan;revenue;costs;profit;solvetime\n";

	const auto stringify_i = [](int i) { return to_string(i); };
	const auto stringify_f = [](float f) { return to_string(f); };

	const auto construct_csv_line = [&p,&stringify_f,&stringify_i](const Runners::GAResult &res, int val) {
		int ms = p.makespan(res.sts);

		vector<int> ints = { val, ms };
		vector<float> floats = { p.revenue[ms], p.totalCosts(res.sts), p.calcProfit(res.sts), static_cast<float>(res.solvetime) };

		vector<string> strs(ints.size()+floats.size());
		const auto lastElemIt = transform(ints.begin(), ints.end(), strs.begin(), stringify_i);
		transform(floats.begin(), floats.end(), lastElemIt, stringify_f);

		return boost::join(strs, ";") + "\n";
	};

	for(int val = from; val <= to; val += step) {
		const auto res = Runners::run(p, params, 4);
		ostr += construct_csv_line(res, val);
	}

	Utils::spit(ostr, csvFilename);
}

void Main::Testing::tweakGAParameters(const string& projFilename, int iterLimit, GAConfigurationExperiment &experiment) {
	ProjectWithOvertime p(projFilename);

	if (computeMinMaxMakespanDifference(p) <= 0) {
		cout << "maxMs - minMs <= 0... ---> skipping!" << endl;
		return;
	}
		
	GAParameters params;

	map<string, int*> nameToPtr = {
		{"popSize", &params.popSize },
		{"numGens", &params.numGens},
		{"pmutate", &params.pmutate},
		{"partitionSize", &params.partitionSize}
	};

	params.rbbrs = true;
	params.enforceTopOrdering = true;
	params.fitnessBasedPairing = false;
	params.traceobj = false;

	params.popSize = 80;
	params.timeLimit = -1;
	params.iterLimit = iterLimit;
	params.numGens = -1;	
	params.pmutate = 5;
	params.selectionMethod = SelectionMethod::BEST;
	params.partitionSize = 4;

	collectResultsVaryingParam(p, params, *nameToPtr[experiment.parameterName], experiment.lb, experiment.ub, experiment.step, "varying_"+experiment.parameterName+".csv");
}
