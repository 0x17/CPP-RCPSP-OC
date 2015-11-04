//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"
#include "../Project.h"

vector<int> Sampling::regretBasedBiasedRandomSampling(Project &p) {
    vector<int> order(p.numJobs);
    vector<bool> jobEligible(p.numJobs);
    jobEligible[0] = true;

    for(int i=0; i<p.numJobs; i++) {
        // choose from set of eligible jobs with p induces by rbbrs
        // set all successors of selected job with no other preds to eligible
    }

    return order;
}

vector<int> Sampling::naiveSampling(Project& p){
    return p.topOrder;
}
