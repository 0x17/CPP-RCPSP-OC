#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "Visualization.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "GeneticAlgorithms/TimeWindow.h"
#include "Stopwatch.h"
#include "GeneticAlgorithms/Runners.h"

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

    ProjectWithOvertime p("../../Projekte/j30/j301_1.sm");
    //Visualization::drawActivityOnNodeGraphToPDF(p, "j301_1.pdf");

    //ProjectWithOvertime p("QBWLBeispiel.DAT");

    //p.reorderDispositionMethod();
    //GARunners::runAll(p, params);

    GARunners::runRange(p, params, 0, 4);

	//Utils::serializeSchedule(result.sts, "myschedulebiatch.txt");
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");
    return 0;
}
