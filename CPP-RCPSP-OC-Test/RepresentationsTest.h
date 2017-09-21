//
// Created by André Schnabel on 06.02.16.
//

#pragma once

#include <gtest/gtest.h>

#include "../GeneticAlgorithms/Representations.h"
#include "../ProjectWithOvertime.h"
#include "TestHelpers.h"

class RepresentationBaseTest : public testing::Test {
protected:
    std::unique_ptr<ProjectWithOvertime> p;

    void SetUp() override {
        p = std::make_unique<ProjectWithOvertime>("Data/MiniBeispiel.DAT");
    }
};

class LambdaTest : public RepresentationBaseTest {
protected:
    std::unique_ptr<Lambda> l;

    void SetUp() override {
        RepresentationBaseTest::SetUp();
        l = std::make_unique<Lambda>(p->numJobs);
        l->order = p->topOrder;
    }

    void TearDown() override {}
};

class LambdaZrtTest : public RepresentationBaseTest {
protected:
    std::unique_ptr<LambdaZrt> lzrt;

    void SetUp() override {
        RepresentationBaseTest::SetUp();
        lzrt = std::make_unique<LambdaZrt>(p->numJobs, p->numRes, p->numPeriods);
        lzrt->order = p->topOrder;
        lzrt->z.foreachAssign([](int r, int t) { return 0; });
    }
};