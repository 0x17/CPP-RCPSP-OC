#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/Runners.h"
#include "BranchAndBound.h"

void convertArgFileToLSP(int argc, char * argv[]) {
    if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, "LSInstance.txt");
	}
}

int main(int argc, const char * argv[]) {
	GAParameters params;
    params.popSize = 80;
    params.numGens = 100;
    params.pmutate = 5;
    params.timeLimit = -1.0;
	
	string pfilename = "/Users/andreschnabel/DropboxLink/Arbeit/Scheduling/Projekte/j30/j301_1.sm";
	//string pfilename = "QBWLBeispiel.DAT";
    ProjectWithOvertime p(pfilename);

	BranchAndBound bb(p);
	auto sts = bb.solve(true);

	//Utils::serializeSchedule(sts, "myschedulebiatch.txt");
	//system(("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe " + pfilename + " myschedulebiatch.txt").c_str());

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

    //GARunners::runRange(p, params, 0, 4);
		
    return 0;
}
