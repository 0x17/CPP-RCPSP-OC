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
	//string projFilename = "../../Projekte/j30/j301_1.sm";
	string projFilename = "QBWLBeispiel.DAT";
	ProjectWithOvertime p(projFilename);
	auto sts = LSSolver::solve(p);
	Utils::serializeSchedule(sts, "myschedulebiatch.txt");
	system("C:\\Users\\a.schnabel\\Dropbox\\Arbeit\\Scheduling\\Code\\ScheduleVisualizer\\ScheduleVisualizerCommand.exe QBWLBeispiel.DAT myschedulebiatch.txt");
    return 0;
}
