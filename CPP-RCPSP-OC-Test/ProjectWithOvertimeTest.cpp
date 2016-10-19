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

TEST_F(ProjectWithOvertimeTest, testBorderSchedulingOptionsSetFromIndex) {
	Matrix<int> expectedTable({
		{ 0, 0, 0 },
		{ 0, 0, 1 },
		{ 0, 1, 0 },
		{ 0, 1, 1 },
		{ 1, 0, 0 },
		{ 1, 0, 1 },
		{ 1, 1, 0 },
		{ 1, 1, 1 }
	});

	ProjectWithOvertime::BorderSchedulingOptions options;
	
	for(int i=0; i<pow(2, 3); i++) {
		options.setFromIndex(i);
		vector<bool> vals = { options.separateCrossover, options.assocIndex, options.upper };
		for(int j=0; j<3; j++)
			ASSERT_TRUE(!expectedTable(i, j) || vals[j]);
	}
}