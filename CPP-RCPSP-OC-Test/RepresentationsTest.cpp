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