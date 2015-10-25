#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"
#include "ScheduleVisualizer.h"

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("QBWLBeispiel.DAT");
    auto optimalSts = LSSolver::solve(p);
    ScheduleVisualizer::drawScheduleToPDF(p, optimalSts, "out.pdf");
    return 0;
}
