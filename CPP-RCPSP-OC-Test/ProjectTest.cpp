//
// Created by André Schnabel on 28.01.16.
//

#include "ProjectTest.h"
#include "TestHelpers.h"

TEST_F(ProjectTest, testConstructor) {
    ASSERT_EQ(5, p->numJobs);
    ASSERT_EQ(7, p->numPeriods);
    ASSERT_EQ(1, p->numRes);

    vector<int> expDurations = { 0, 2, 2, 2, 0 };
    TestHelpers::arrayEquals(expDurations, p->durations);

	vector<int> expDemands = { 0, 1, 1, 2, 0 };
	vector<int> actDemands = p->demands.column(0);
	TestHelpers::arrayEquals(expDemands, actDemands);

	vector<vector<char>> expAdjVecs = {
            { 0, 1, 0, 1, 0 },
            { 0, 0, 1, 0, 0 },
            { 0, 0, 0, 0, 1 },
            { 0, 0, 0, 0, 1 },
            { 0, 0, 0, 0, 0 }
    };
    Matrix<char> expAdjMx(expAdjVecs);
    TestHelpers::matrixEquals(expAdjMx, p->adjMx);

    ASSERT_EQ(1, p->capacities.size());
    ASSERT_EQ(2, p->capacities[0]);

    ASSERT_EQ(p->T, 6);
}

TEST_F(ProjectTest, testSerialSGS) {
    auto actualSts = p->serialSGS({0, 1, 2, 3, 4});
    vector<int> expSts = { 0, 0, 2, 4, 6 };
    TestHelpers::arrayEquals(expSts, actualSts);
}

TEST_F(ProjectTest, testJobBeforeInOrder) {
    vector<int> order = { 0, 1, 2, 3, 4, 5 };
    ASSERT_TRUE(p->jobBeforeInOrder(1, 2, order));
    ASSERT_FALSE(p->jobBeforeInOrder(1, 1, order));
    ASSERT_TRUE(p->jobBeforeInOrder(4, 5, order));
    ASSERT_FALSE(p->jobBeforeInOrder(5, 5, order));
    for(int i=1; i<order.size(); i++)
        ASSERT_TRUE(p->jobBeforeInOrder(0, i, order));
}

TEST_F(ProjectTest, testHasPredNotBeforeInOrder) {
    vector<int> order = { 0, 1, 2, 3, 4 };
    ASSERT_TRUE(p->hasPredNotBeforeInOrder(4, 0, order));
    ASSERT_FALSE(p->hasPredNotBeforeInOrder(4, 4, order));
    ASSERT_FALSE(p->hasPredNotBeforeInOrder(2, 2, order));
}

TEST_F(ProjectTest, testMakespan) {
    vector<int> sts = { 0, 0, 2, 4, 6 };
    ASSERT_EQ(6, p->makespan(sts));
}

TEST_F(ProjectTest, testEachJob) {
    list<int> jobs;
    p->eachJob([&](int j) { jobs.push_back(j); });
    ASSERT_EQ(p->numJobs, jobs.size());
    int i;
    list<int>::iterator jit;
    for(i=0, jit = jobs.begin(); i<p->numJobs; i++, ++jit)
        ASSERT_EQ(i, (*jit));
}
