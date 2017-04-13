//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"
#include "../Project.h"
#include <algorithm>
#include <numeric>

vector<float> Sampling::computeProbsForDecisionSet(const vector<bool> &eligible, const vector<float> &priorityValues) {
    int len = static_cast<int>(priorityValues.size());
    float minPrio = std::numeric_limits<float>::max(); // use filter iterator from boost instead?
    for(int j = 0; j < len; j++) {
        if (eligible[j] && priorityValues[j] < minPrio)
            minPrio = priorityValues[j];
    }

    vector<float> w(len, 0.0f);
    float wsum = 0.0f;
    for (int j = 0; j < len; j++) {
        if(eligible[j]) {
            w[j] = priorityValues[j] - minPrio;
            wsum += w[j] + 1.0f;
        }
    }

    vector<float> probs(len, 0.0f);
    for(int j = 0; j <len; j++) {
        if(eligible[j])
            probs[j] = (w[j] + 1.0f) / wsum;
    }

    return probs;
}

int Sampling::pickFromDecisionSet(const vector<bool> &eligible, const vector<float> &priorityValues) {
    auto probs = computeProbsForDecisionSet(eligible, priorityValues);
	return Utils::pickWithDistribution(probs);
}

vector<int> Sampling::regretBasedBiasedRandomSampling(const Project &p, const vector<int> &priorityValues) {
	vector<float> pvalsFloat = Utils::mapVec<float(int), int, float>([](int pval) { return static_cast<float>(pval); }, priorityValues);
	return regretBasedBiasedRandomSampling(p, pvalsFloat);
}

vector<int> Sampling::regretBasedBiasedRandomSamplingInv(const Project &p, const vector<int> &priorityValues) {
	vector<float> pvalsFloat = Utils::mapVec<float(int), int, float>([](int pval) { return -static_cast<float>(pval); }, priorityValues);
	return regretBasedBiasedRandomSampling(p, pvalsFloat);
}

void Sampling::updateEligible(const Project &p, const vector<int> &order, int curIndex, vector<bool> &eligible) {
	eligible[order[curIndex]] = false;
	for (int j = 0; j < p.numJobs; j++)
		if (p.adjMx(order[curIndex], j))
			eligible[j] = !p.hasPredNotBeforeInOrder(j, curIndex+1, order);
}

vector<int> Sampling::regretBasedBiasedRandomSampling(const Project &p, const vector<float> &priorityValues) {
	vector<bool> eligible(p.numJobs, false);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for (int i = 0; i < p.numJobs; i++) {
		order[i] = pickFromDecisionSet(eligible, priorityValues);
		updateEligible(p, order, i, eligible);
	}

	return order;
}

vector<int> Sampling::naiveSampling(const Project& p) {
	vector<bool> eligible(p.numJobs, false);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for(int i = 0; i < p.numJobs; i++) {
		int nth = Utils::randRangeIncl(0, static_cast<int>(count(eligible.begin(), eligible.end(), true))-1);
		order[i] = Utils::indexOfNthEqualTo(nth, true, eligible);
		updateEligible(p, order, i, eligible);
	}
	return order;
}

vector<int> Sampling::regretBasedBiasedRandomSamplingForLfts(const Project &p){
    return regretBasedBiasedRandomSamplingInv(p, p.lfts);
}

vector<int> Sampling::sample(bool rbbrs, const Project &p) {
    return rbbrs ? Sampling::regretBasedBiasedRandomSamplingForLfts(p) : Sampling::naiveSampling(p);
}