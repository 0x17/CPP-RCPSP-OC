//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_VISUALIZATION_H
#define CPP_RCPSP_OC_VISUALIZATION_H

#include "ProjectWithOvertime.h"

namespace Visualization {
    void drawActivityOnNodeGraphToPDF(Project &p, string filename);
    void drawScheduleToPDF(ProjectWithOvertime &p, vector<int> sts, string filename);
    string activityOnNodeGraphDOT(Project &p);
};

#endif //CPP_RCPSP_OC_SCHEDULEVISUALIZER_H
