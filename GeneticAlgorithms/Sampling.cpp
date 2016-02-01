//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"
#include "../Project.h"
#include <algorithm>

vector<float> Sampling::computeProbsForDecisionSet(vector<bool> &eligible, vector<float> &priorityValues) {
    int len = static_cast<int>(priorityValues.size());
    float minPrio = numeric_limits<float>::max(); // use filter iterator from boost instead?
    for(int i = 0; i < len; i++) {
        if (eligible[i] && priorityValues[i] < minPrio)
            minPrio = priorityValues[i];
    }

    vector<float> w(len, 0.0f);
    float wsum = 0.0f;
    for (int i = 0; i < len; i++) {
        if(eligible[i]) {
            w[i] = priorityValues[i] - minPrio;
            wsum += w[i] + 1;
        }
    }

    vector<float> probs(len, 0.0f);
    for(int j = 0; j <len; j++) {
        if(eligible[j])
            probs[j] = 1 / wsum * (w[j] + 1);
    }

    return probs;
}

int Sampling::pickFromDecisionSet(vector<bool> &eligible, vector<float> &priorityValues) {
    auto probs = computeProbsForDecisionSet(eligible, priorityValues);
	return Utils::pickWithDistribution(probs);
}

vector<int> Sampling::regretBasedBiasedRandomSampling(Project &p, vector<float> &priorityValues) {
	vector<bool> eligible(p.numJobs);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for (int i = 0; i < p.numJobs; i++) {
		order[i] = pickFromDecisionSet(eligible, priorityValues);
        p.eachJob([&](int j){ eligible[j] = !p.jobBeforeInOrder(j, i + 1, order) && !p.hasPredNotBeforeInOrder(j, i + 1, order); });
	}

	return order;
}

vector<int> Sampling::naiveSampling(Project& p) {
	vector<bool> eligible(p.numJobs);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for(int i = 0; i < p.numJobs; i++) {
		int nth = Utils::randRangeIncl(0, static_cast<int>(count(eligible.begin(), eligible.end(), true))-1);
		order[i] = Utils::indexOfNthEqualTo(nth, true, eligible);
        p.eachJob([&](int j) { eligible[j] = !p.jobBeforeInOrder(j, i+1, order) && !p.hasPredNotBeforeInOrder(j, i+1, order); });
	}
	return order;
}
