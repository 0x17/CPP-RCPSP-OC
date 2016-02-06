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
    Lambda *l;
    ProjectWithOvertime *p;

    void SetUp() override {
        p = new ProjectWithOvertime("MiniBeispiel.DAT");
        l = new Lambda(p->numJobs);
        l->order = p->topOrder;
    }

    void TearDown() override {
        delete l;
        delete p;
    }
};


#endif //CPP_RCPSP_OC_REPRESENTATIONSTEST_H
