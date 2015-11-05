#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "ScheduleVisualizer.h"
#include "GeneticAlgorithms/OvertimeBound.h"

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("QBWLBeispiel.DAT");
	FixedCapacityGA ga(p);
	auto pair = ga.solve();
	/*if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, "LSInstance.txt");
	}*/
    //auto optimalSts = LSSolver::solveMIPStyle(p);
    //auto optimalSts = LSSolver::solve(p);
	//auto optimalSts = LSSolver::solve2(p);
	//auto optimalSts = LSSolver::solve3(p);
	Utils::serializeSchedule(pair.first, "myschedulebiatch.txt");
    //vector<int> optimalSts(p.numJobs);
    //ScheduleVisualizer::drawScheduleToPDF(p, optimalSts, "out.pdf");
	//cin.get();
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");
    return 0;
}
