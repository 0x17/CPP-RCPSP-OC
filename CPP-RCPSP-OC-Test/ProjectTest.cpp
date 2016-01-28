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
    TestHelpers::ArrayEquals(expDurations, p->durations);

    // TODO: Finish
}