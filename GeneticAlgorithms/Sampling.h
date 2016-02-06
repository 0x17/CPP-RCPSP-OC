//
// Created by André Schnabel on 31.10.15.
//

#ifndef CPP_RCPSP_OC_SAMPLING_H
#define CPP_RCPSP_OC_SAMPLING_H


#include "../Project.h"

namespace Sampling {
    vector<int> naiveSampling(Project &p);
    vector<int> regretBasedBiasedRandomSamplingForLfts(Project &p);
    vector<int> regretBasedBiasedRandomSampling(Project &p, vector<float> &priorityValues);
	vector<int> regretBasedBiasedRandomSampling(Project &p, vector<int> &priorityValues);
	vector<int> regretBasedBiasedRandomSamplingInv(Project &p, vector<int> &priorityValues);

    int pickFromDecisionSet(vector<bool> &eligible, vector<float> &priorityValues);
    vector<float> computeProbsForDecisionSet(vector<bool> &eligible, vector<float> &priorityValues);

    vector<int> sample(bool rbbrs, Project &p);
};


#endif //CPP_RCPSP_OC_SAMPLING_H
