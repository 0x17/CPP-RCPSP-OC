//
// Created by André Schnabel on 31.10.15.
//

#pragma once

#include "../Project.h"

namespace Sampling {
    vector<int> naiveSampling(const Project &p);

    vector<int> regretBasedBiasedRandomSamplingForLfts(const Project &p);
    vector<int> regretBasedBiasedRandomSampling(const Project &p, const vector<float> &priorityValues);
	vector<int> regretBasedBiasedRandomSampling(const Project &p, const vector<int> &priorityValues);
	vector<int> regretBasedBiasedRandomSamplingInv(const Project &p, const vector<int> &priorityValues);

    int pickFromDecisionSet(const vector<bool> &eligible, const vector<float> &priorityValues);
    vector<float> computeProbsForDecisionSet(const vector<bool> &eligible, const vector<float> &priorityValues);

	void updateEligible(const Project &p, const vector<int> &order, int curIndex, vector<bool> &eligible);

    vector<int> sample(bool rbbrs, const Project &p);
};
