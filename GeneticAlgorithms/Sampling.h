//
// Created by André Schnabel on 31.10.15.
//

#ifndef CPP_RCPSP_OC_SAMPLING_H
#define CPP_RCPSP_OC_SAMPLING_H


#include "../Project.h"

namespace Sampling {
    vector<int> naiveSampling(Project &p);
    vector<int> regretBasedBiasedRandomSampling(Project &p);
};


#endif //CPP_RCPSP_OC_SAMPLING_H