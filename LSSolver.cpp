//
// Created by André Schnabel on 24.10.15.
//

#include <localsolver.h>
#include "LSSolver.h"
#include "ProjectWithOvertime.h"

using namespace localsolver;

vector<int> LSSolver::solve(ProjectWithOvertime &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    vector<vector<LSExpression>> xjt = Utils::initMatrix<LSExpression>(p.numJobs, p.numPeriods);
    EACH_COMMON(j, p.numJobs, EACH_COMMON(t, p.numPeriods, model.boolVar()))

    vector<vector<LSExpression>> zrt = Utils::initMatrix<LSExpression>(p.numRes, p.numPeriods);
    EACH_COMMON(r, p.numRes, EACH_COMMON(t, p.numPeriods, model.intVar(0, p.zmax[r])))

    // TODO: Finish...


    return sts;
}