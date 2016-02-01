//
// Created by André Schnabel on 01.02.16.
//

#include "SamplingTest.h"
#include <gtest/gtest.h>
#include <numeric>
#include "../Project.h"
#include "../GeneticAlgorithms/Sampling.h"

template<class Func>
void testSamplingCommon(Func samplingFunc) {
    Project p("MiniBeispiel.DAT");
    vector<vector<int>> orders(10);
    for(int i=0; i<10; i++) {
        orders[i] = samplingFunc(p);
        ASSERT_TRUE(p.isOrderFeasible(orders[i]));
    }
    ASSERT_TRUE(any_of(orders.begin(), orders.end(), [&p](vector<int> &order) { return !equal(order.begin(), order.end(), p.topOrder.begin(), p.topOrder.end()); }));
}

vector<float> lftsToFloat(Project &p) {
    vector<float> lftsFloat = Utils::mapVec<float(int), int, float>([](int lft) { return static_cast<float>(lft); }, p.lfts);
    return lftsFloat;
}

TEST(SamplingTest, testPickFromDecisionSet) {
}

TEST(SamplingTest, testComputeProbsForDecisionSet) {
    Project p("MiniBeispiel.DAT");
    vector<float> lftsFloat = lftsToFloat(p);
    vector<bool> eligs(p.numJobs, false);
    eligs[0] = true;
    auto probs = Sampling::computeProbsForDecisionSet(eligs, lftsFloat);
    ASSERT_EQ(1.0f, accumulate(probs.begin(), probs.end(), 0.0f));
    ASSERT_TRUE(probs[0] > 0.0f);
    for(int i=1; i<p.numJobs; i++) {
        ASSERT_EQ(0.0f, probs[i]);
    }
}

TEST(SamplingTest, testNaiveSampling) {
    testSamplingCommon(&Sampling::naiveSampling);
}

TEST(SamplingTest, testRegretBasedBiasedRandomSampling) {
    auto sfunc = [](Project &p) {
        vector<float> lftsFloat = lftsToFloat(p);
        return Sampling::regretBasedBiasedRandomSampling(p, lftsFloat);
    };
    testSamplingCommon(sfunc);
}
