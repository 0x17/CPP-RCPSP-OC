//
// Created by André Schnabel on 02.02.16.
//

#include "ProjectWithOvertimeTest.h"
#include "TestHelpers.h"

TEST_F(ProjectWithOvertimeTest, testChooseEligibleWithLowestIndex) {
    vector<int> sts(p->numJobs, Project::UNSCHEDULED);
    sts[0] = 0;
    vector<int> order = { 0, 3, 1, 2, 4 };
    ASSERT_EQ(3, p->chooseEligibleWithLowestIndex(sts, order));
    order = { 0, 2, 1, 3, 4 };
    ASSERT_EQ(1, p->chooseEligibleWithLowestIndex(sts, order));
    sts[1] = 0;
    ASSERT_EQ(2, p->chooseEligibleWithLowestIndex(sts, order));
}

TEST_F(ProjectWithOvertimeTest, testSerialSGSWithDeadline) {
	auto res = p->serialSGSWithDeadline(p->numPeriods, p->topOrder);
	ASSERT_TRUE(res.first);
}

TEST_F(ProjectWithOvertimeTest, testDecisionTimesForResDevProblem) {
	vector<int> sts = p->emptySchedule();	
	Matrix<int> resRem = p->normalCapacityProfile();

	auto ests = p->earliestStartingTimesForPartial(sts);
	auto lfts = p->latestFinishingTimesForPartial(sts, p->numPeriods);

	auto dtimes = p->decisionTimesForResDevProblem(sts, ests, lfts, resRem, 0);
	ASSERT_FALSE(dtimes.empty());
	list<int> expTimes = { 0, 3 };
	TestHelpers::listEquals(expTimes, dtimes);
}