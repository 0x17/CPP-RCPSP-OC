#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "ScheduleVisualizer.h"
#include "GeneticAlgorithms/OvertimeBound.h"
#include "Stopwatch.h"

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("../../Projekte/j120/j1201_1.sm");
	FixedCapacityGA ga(p);
	
	Stopwatch sw;
	sw.start();
	auto pair = ga.solve();
	double solvetime = sw.look();
	cout << endl << "Solvetime = " << solvetime << " ms" << endl;

	/*if (argc == 2) {
		ProjectWithOvertime p(argv[1]);
		LSSolver::writeLSPModelParamFile(p, "LSInstance.txt");
	}*/
    //auto optimalSts = LSSolver::solveMIPStyle(p);
    //auto optimalSts = LSSolver::solve(p);
	//auto optimalSts = LSSolver::solve2(p);
	//auto optimalSts = LSSolver::solve3(p);
	//Utils::serializeSchedule(pair.first, "myschedulebiatch.txt");
    //vector<int> optimalSts(p.numJobs);
    //ScheduleVisualizer::drawScheduleToPDF(p, optimalSts, "out.pdf");
	//cin.get();
	//system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Projekte\\j30\\j301_1.sm myschedulebiatch.txt");
    return 0;
}
