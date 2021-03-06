//
// Created by André Schnabel on 01.02.16.
//

#include <gtest/gtest.h>
#include <numeric>
#include "../Project.h"
#include "../GeneticAlgorithms/Sampling.h"
#include "ProjectTest.h"
#include "TestHelpers.h"

using namespace std;

class SamplingTest : public ProjectTest {};

void testSamplingCommon(Project &p, const std::function<vector<int>(const Project &p)> samplingFunc) {
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

TEST_F(SamplingTest, testPickFromDecisionSet) {
    vector<float> lftsFloat = lftsToFloat(*p);
    vector<bool> eligs(p->numJobs, false);
    eligs[0] = true;
    ASSERT_EQ(0, Sampling::pickFromDecisionSet(eligs, lftsFloat));
}

TEST_F(SamplingTest, testComputeProbsForDecisionSet) {
    vector<float> lftsFloat = lftsToFloat(*p);
    vector<bool> eligs(p->numJobs, false);
    eligs[0] = true;
    auto probs = Sampling::computeProbsForDecisionSet(eligs, lftsFloat);
    ASSERT_EQ(1.0f, accumulate(probs.begin(), probs.end(), 0.0f));
    ASSERT_TRUE(probs[0] > 0.0f);
    for(int i=1; i<p->numJobs; i++) {
        ASSERT_EQ(0.0f, probs[i]);
    }

	eligs[0] = false;
	eligs[1] = true;
	eligs[3] = true;
	probs = Sampling::computeProbsForDecisionSet(eligs, lftsFloat);
	ASSERT_EQ(1.0f, accumulate(probs.begin(), probs.end(), 0.0f));
}

TEST_F(SamplingTest, testNaiveSampling) {
    testSamplingCommon(*p, &Sampling::naiveSampling);
}

TEST_F(SamplingTest, testRegretBasedBiasedRandomSampling) {
    auto sfunc = [](const Project &p) {
        return Sampling::regretBasedBiasedRandomSampling(p, p.lfts);
    };
    testSamplingCommon(*p, sfunc);
}

TEST_F(SamplingTest, testUpdateEligible) {
	vector<int> order = { 0, 1, 2, 3, 4 };
	vector<bool>	eligible(5, false),
					eligibleExp = { false, true, false, true, false };
	eligible[0] = true;
	Sampling::updateEligible(*p, order, 0, eligible);
	TestHelpers::arrayEquals(eligibleExp, eligible);
}