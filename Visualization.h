//
// Created by André Schnabel on 25.10.15.
//

#pragma once

#include "ProjectWithOvertime.h"

namespace Visualization {
    void drawActivityOnNodeGraphToPDF(Project &p, string filename);
    void drawScheduleToPDF(ProjectWithOvertime &p, vector<int> sts, string filename);
    string activityOnNodeGraphDOT(Project &p);
};

