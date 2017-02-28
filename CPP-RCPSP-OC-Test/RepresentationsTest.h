//
// Created by André Schnabel on 06.02.16.
//

#ifndef CPP_RCPSP_OC_REPRESENTATIONSTEST_H
#define CPP_RCPSP_OC_REPRESENTATIONSTEST_H

#include <gtest/gtest.h>

#include "../GeneticAlgorithms/Representations.h"
#include "../ProjectWithOvertime.h"
#include "TestHelpers.h"

class LambdaTest : public testing::Test {
protected:
    unique_ptr<Lambda> l;
    unique_ptr<ProjectWithOvertime> p;

    void SetUp() override {
        p = make_unique<ProjectWithOvertime>("MiniBeispiel.DAT");
        l = make_unique<Lambda>(p->numJobs);
        l->order = p->topOrder;
    }

    void TearDown() override {
    }
};


#endif //CPP_RCPSP_OC_REPRESENTATIONSTEST_H
