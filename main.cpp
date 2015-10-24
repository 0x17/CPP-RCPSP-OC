// TODO: LocalSolver einbinden und mit kompakter logischer Notation lösen lassen

#include <iostream>
#include "ProjectWithOvertime.h"
#include "LSSolver.h"

int main(int argc, const char * argv[]) {
    ProjectWithOvertime p("QBWLBeispiel.DAT");
    cout << "Number of jobs " << p.numJobs << endl;
    return 0;
}
