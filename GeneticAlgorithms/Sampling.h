//
// Created by André Schnabel on 31.10.15.
//

#pragma once

#include "../Project.h"

namespace Sampling {
    std::vector<int> naiveSampling(const Project &p);

    std::vector<int> regretBasedBiasedRandomSamplingForLfts(const Project &p);
    std::vector<int> regretBasedBiasedRandomSampling(const Project &p, const std::vector<float> &priorityValues);
	std::vector<int> regretBasedBiasedRandomSampling(const Project &p, const std::vector<int> &priorityValues);
	std::vector<int> regretBasedBiasedRandomSamplingInv(const Project &p, const std::vector<int> &priorityValues);

    int pickFromDecisionSet(const std::vector<bool> &eligible, const std::vector<float> &priorityValues);
    std::vector<float> computeProbsForDecisionSet(const std::vector<bool> &eligible, const std::vector<float> &priorityValues);

	void updateEligible(const Project &p, const std::vector<int> &order, int curIndex, std::vector<bool> &eligible);

    std::vector<int> sample(bool rbbrs, const Project &p);
};
