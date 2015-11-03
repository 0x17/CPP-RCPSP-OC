//
// Created by André Schnabel on 31.10.15.
//

#include "Sampling.h"

vector<int> Sampling::regretBasedBiasedRandomSampling(Project &p) {
    vector<int> order;
    vector<bool> jobEligible;

    jobEligible[0] = true;

    for(int i=0; i<p.numJobs; i++) {
        // choose from set of eligible jobs with p induces by rbbrs
        // set all successors of selected job with no other preds to eligible
    }

    return order;
}

void makeSuccsWithoutOtherPredsEligible(Project &p, int j, vector<bool> &eligibles, int &numEligibles) {
    EACH_RNG(k, p.numJobs,
        if(p.adjMx[j][k]) {
            bool hasOtherPred = false;
            for(int l=0; l<p.numJobs; l++) {
                if(p.adjMx[l][k] && l != j) {
                    hasOtherPred = true;
                    break;
                }
            }
            if(!hasOtherPred) {
                eligibles[k] = true;
                numEligibles++;
            }
        }
    )
}

int nthEligibleJob(Project &p, int q, vector<bool> &eligibles, int &numEligibles) {
    int nth = 0;
    EACH_RNG(j, p.numJobs,
        if(eligibles[j]) {
            if(nth == q) {
                makeSuccsWithoutOtherPredsEligible(p, j, eligibles, numEligibles);
                return j;
            }
            nth++;
        }
    )
    return -1;
}

vector<int> Sampling::naiveSampling(Project& p){
    vector<int> order;

    vector<bool> eligibles;
    eligibles[0] = true;
    int numEligibles = 1;

    EACH_RNG(i, p.numJobs,
        int q = Utils::randRangeIncl(0, numEligibles-1);
        order[i] = nthEligibleJob(p, q, eligibles, numEligibles);
        numEligibles--;
    )

    return order;
}
