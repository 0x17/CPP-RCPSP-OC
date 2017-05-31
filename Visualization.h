//
// Created by André Schnabel on 25.10.15.
//

#pragma once

#include "ProjectWithOvertime.h"

namespace Visualization {
    void drawActivityOnNodeGraphToPDF(Project &p, std::string filename);
    void drawScheduleToPDF(ProjectWithOvertime &p, std::vector<int> sts, std::string filename);
	std::string activityOnNodeGraphDOT(Project &p);
};

