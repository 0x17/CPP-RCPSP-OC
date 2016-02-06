//
// Created by André Schnabel on 06.02.16.
//

#include "RepresentationsTest.h"

TEST_F(LambdaTest, testOnePointCrossover) {
    Lambda m(p->numJobs), f(p->numJobs);
    m.order = { 0, 1, 2, 3, 4 };
    f.order = { 0, 1, 2, 3, 4 };
    l->onePointCrossover(m, f, 1);
    Lambda expDaughter({ 0, 1, 2, 3, 4 });
    TestHelpers::arrayEquals(expDaughter.order, l->order);
}