// TODO: LocalSolver einbinden und mit kompakter logischer Notation lösen lassen

#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("QBWLBeispiel.DAT");
    auto optimalSts = LSSolver::solve(p);
    return 0;
}
