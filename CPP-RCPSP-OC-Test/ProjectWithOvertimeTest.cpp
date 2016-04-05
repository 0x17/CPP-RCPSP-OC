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
	auto res = p->serialSGSWithDeadlineEarly(p->numPeriods-1, p->topOrder);
	ASSERT_TRUE(res.first);
}

TEST_F(ProjectWithOvertimeTest, testDecisionTimesForResDevProblem) {
	vector<int> sts = p->emptySchedule();	
	Matrix<int> resRem = p->normalCapacityProfile();

	auto ests = p->earliestStartingTimesForPartial(sts);
	auto lfts = p->latestFinishingTimesForPartial(sts, p->numPeriods);

	auto dtimes = p->decisionTimesForResDevProblem(sts, ests, lfts, resRem, 0);
	ASSERT_FALSE(dtimes.empty());
	vector<int> expTimes = { 0, 3 };
	TestHelpers::arrayEquals(expTimes, dtimes);
}

TEST_F(ProjectWithOvertimeTest, testEnoughCapacityForJobWithBaseInterval) {
	auto sts = p->emptySchedule();
	auto cests = p->earliestStartingTimesForPartial(sts);
	auto clfts = p->latestFinishingTimesForPartial(sts, 4);
	auto resRem = p->normalCapacityProfile();

	ASSERT_TRUE(p->enoughCapacityForJobWithBaseInterval(sts, cests, clfts, resRem, 0, 0));

	sts = { 0, Project::UNSCHEDULED, 2, Project::UNSCHEDULED, 4 };
	cests = p->earliestStartingTimesForPartial(sts);
	clfts = p->latestFinishingTimesForPartial(sts, 4);
	resRem = p->resRemForPartial(sts);

	ASSERT_TRUE(p->enoughCapacityForJobWithBaseInterval(sts, cests, clfts, resRem, 3, 0));

	sts = { 0, Project::UNSCHEDULED, 2, 0, 4 };
	cests = p->earliestStartingTimesForPartial(sts);
	clfts = p->latestFinishingTimesForPartial(sts, 4);
	resRem = p->resRemForPartial(sts);

	ASSERT_FALSE(p->enoughCapacityForJobWithBaseInterval(sts, cests, clfts, resRem, 1, 0));
}