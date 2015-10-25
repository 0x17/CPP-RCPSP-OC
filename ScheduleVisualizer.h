//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_SCHEDULEVISUALIZER_H
#define CPP_RCPSP_OC_SCHEDULEVISUALIZER_H

#include "ProjectWithOvertime.h"

namespace ScheduleVisualizer {
    void drawScheduleToPDF(ProjectWithOvertime &p, vector<int> sts, string filename);

};


#endif //CPP_RCPSP_OC_SCHEDULEVISUALIZER_H
