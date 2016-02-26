//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"
#include "../Project.h"
#include <algorithm>
#include <numeric>

vector<float> Sampling::computeProbsForDecisionSet(vector<bool> &eligible, vector<float> &priorityValues) {
    int len = static_cast<int>(priorityValues.size());
    float minPrio = numeric_limits<float>::max(); // use filter iterator from boost instead?
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

int Sampling::pickFromDecisionSet(vector<bool> &eligible, vector<float> &priorityValues) {
    auto probs = computeProbsForDecisionSet(eligible, priorityValues);
	return Utils::pickWithDistribution(probs);
}

vector<int> Sampling::regretBasedBiasedRandomSampling(Project &p, vector<int> &priorityValues) {
	vector<float> pvalsFloat = Utils::mapVec<float(int), int, float>([](int pval) { return static_cast<float>(pval); }, priorityValues);
	return regretBasedBiasedRandomSampling(p, pvalsFloat);
}

vector<int> Sampling::regretBasedBiasedRandomSamplingInv(Project &p, vector<int> &priorityValues) {
	vector<float> pvalsFloat = Utils::mapVec<float(int), int, float>([](int pval) { return -static_cast<float>(pval); }, priorityValues);
	return regretBasedBiasedRandomSampling(p, pvalsFloat);
}

void Sampling::updateEligible(Project &p, vector<int> &order, int curIndex, vector<bool> &eligible) {
	eligible[order[curIndex]] = false;
	for (int j = 0; j < p.numJobs; j++)
		if (p.adjMx(order[curIndex], j))
			eligible[j] = !p.hasPredNotBeforeInOrder(j, curIndex+1, order);
}

vector<int> Sampling::regretBasedBiasedRandomSampling(Project &p, vector<float> &priorityValues) {
	vector<bool> eligible(p.numJobs, false);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for (int i = 0; i < p.numJobs; i++) {
		if (i == 5)
			printf("");
		order[i] = pickFromDecisionSet(eligible, priorityValues);
		updateEligible(p, order, i, eligible);
	}

	return order;
}

vector<int> Sampling::naiveSampling(Project& p) {
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

vector<int> Sampling::regretBasedBiasedRandomSamplingForLfts(Project &p){
    return regretBasedBiasedRandomSamplingInv(p, p.lfts);
}

vector<int> Sampling::sample(bool rbbrs, Project &p) {
    return rbbrs ? Sampling::regretBasedBiasedRandomSamplingForLfts(p) : Sampling::naiveSampling(p);
}