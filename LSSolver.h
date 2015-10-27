//
// Created by André Schnabel on 24.10.15.
//

#ifndef CPP_RCPSP_OC_LSSOLVER_H
#define CPP_RCPSP_OC_LSSOLVER_H

#include "Project.h"
#include "ProjectWithOvertime.h"

namespace LSSolver {
    vector<int> solve(ProjectWithOvertime &p);
    vector<int> solveMIPStyle(ProjectWithOvertime &p);
};

#endif //CPP_RCPSP_OC_LSSOLVER_H
