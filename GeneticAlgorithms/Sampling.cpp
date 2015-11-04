//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"
#include "../Project.h"

vector<int> Sampling::regretBasedBiasedRandomSampling(Project& p) {
	vector<int> order;
	return order;
}

vector<int> Sampling::naiveSampling(Project& p) {
	vector<bool> eligible(p.numJobs);
	vector<int> order(p.numJobs);
	eligible[0] = true;
	for(int i = 0; i < p.numJobs; i++) {
		int nth = Utils::randRangeIncl(0, static_cast<int>(count(eligible.begin(), eligible.end(), true))-1);
		order[i] = Utils::indexOfNthEqualTo(nth, true, eligible);
		P_EACH_JOB(eligible[j] = !p.jobBeforeInOrder(j, i, order) && !p.hasPredNotBeforeInOrder(j, i, order))
	}
	return order;
}
