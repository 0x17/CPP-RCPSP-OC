//
// Created by André Schnabel on 06.02.16.
//

#include "RepresentationsTest.h"

using namespace std;

TEST_F(LambdaTest, testOnePointCrossover) {
    Lambda m(p->numJobs), f(p->numJobs);
    m.order = { 0, 1, 2, 3, 4 };
    f.order = { 0, 1, 2, 3, 4 };
    l->onePointCrossover(m, f, 1);
    Lambda expDaughter({ 0, 1, 2, 3, 4 });
    TestHelpers::arrayEquals(expDaughter.order, l->order);
}

TEST_F(LambdaTest, testTwoPointCrossover) {
	Lambda m(p->numJobs), f(p->numJobs);
	m.order = { 0, 1, 3, 2, 4 };
	f.order = { 0, 2, 3, 1, 4 };
	l->twoPointCrossover(m, f, 1, 2);
	Lambda expDaughter({ 0, 1, 2, 3, 4 });
	TestHelpers::arrayEquals(expDaughter.order, l->order);
}

TEST_F(LambdaTest, testNeighborhoodSwap) {
    vector<int> orderBefore = l->order;
    l->neighborhoodSwap(p->adjMx, 1);
    for(int i=0; i<orderBefore.size(); i++) {
        if(l->order[i] != orderBefore[i]) {
            ASSERT_TRUE(i+1 < orderBefore.size());
            ASSERT_TRUE(l->order[i] == orderBefore[i+1]);
            ASSERT_TRUE(l->order[i+1] == orderBefore[i]);
        }
    }
}

//======================================================================================================================

TEST_F(LambdaZrtTest, testIndependentTwoPointCrossover) {
	LambdaZrt m(p->numJobs, p->numRes, p->numPeriods),
			  f(p->numJobs, p->numRes, p->numPeriods);

	m.order = { 0, 1, 3, 2, 4 };
	f.order = { 0, 2, 3, 1, 4 };
	pair<int,int> qj = {1, 2};

	m.z = Matrix<int>({{1,2,3,4,5,6,7}});
	f.z = Matrix<int>({{7,6,5,4,3,2,1}});
	pair<int,int> q2 = {2, 4};
	auto ctype = LambdaZrt::CrossoverPartitionType::PERIOD_WISE;

	lzrt->independentTwoPointCrossovers(m, f, qj, q2, ctype);

	vector<int> expOrder = { 0, 1, 2, 3, 4 };
	Matrix<int> expProfile({{1,2,3,4,3,6,7}});
	LambdaZrt expDaughter(expOrder, expProfile);

	TestHelpers::arrayEquals(expDaughter.order, lzrt->order);
	TestHelpers::matrixEquals(expDaughter.z, lzrt->z);
}