//
// Created by André Schnabel on 24.10.15.
//

#include <localsolver.h>
#include "LSSolver.h"

using namespace localsolver;

vector<int> LSSolver::solve(Project &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    // TODO: Implement me!

    return sts;
}