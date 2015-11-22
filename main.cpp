#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/Runners.h"
#include "BranchAndBound.h"

void convertArgFileToLSP(int argc, const char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, string(argv[1])+".txt");
	}
}

int main(int argc, const char * argv[]) {
	/*GAParameters params;
    params.popSize = 80;
    params.numGens = 100;
    params.pmutate = 5;
    params.timeLimit = -1.0;
	
	string pfilename = "../../Projekte/j30/j301_1.sm";
	//string pfilename = "QBWLBeispiel.DAT";
	//string pfilename = "../../Projekte/j60/j601_1.sm";
    ProjectWithOvertime p(pfilename);

	BranchAndBound bb(p, false);
	auto sts = bb.solve(false);

	//auto result = GARunners::runSpecific(p, params, 2);
	//vector<int> sts = result.sts;

	Utils::serializeSchedule(sts, "myschedulebiatch.txt");
	system(("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe " + pfilename + " myschedulebiatch.txt").c_str());
	
	//auto res = GARunners::runSpecific(p, params, 0);	
	//Utils::serializeSchedule(res.sts, "myschedulebiatch.txt");
	//system(("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe " + pfilename + " myschedulebiatch.txt").c_str());

    //cout << p.calcProfit(sts) << endl;

    //ProjectWithOvertime p("../../Projekte/j30/j301_1.sm");

    //auto res = GARunners::runSpecific(p, params, 0);

    //Visualization::drawScheduleToPDF(p, res.sts, "myschedule.pdf");

    //Visualization::drawActivityOnNodeGraphToPDF(p, "j301_1.pdf");

    //ProjectWithOvertime p("QBWLBeispiel.DAT");

    //p.reorderDispositionMethod();
    //GARunners::runAll(p, params);

    //GARunners::runRange(p, params, 0, 4);*/

	//convertArgFileToLSP(argc, argv);

	auto filenames = Utils::filenamesInDirWithExt(".", ".cpp");
    for(auto filename : filenames)
        cout << "Filename = " << filename << endl;
		
    return 0;
}
