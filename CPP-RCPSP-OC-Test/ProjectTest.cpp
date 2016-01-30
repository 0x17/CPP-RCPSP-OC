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