//
// Created by André Schnabel on 06.02.16.
//

#pragma once

#include <gtest/gtest.h>

#include "../GeneticAlgorithms/Representations.h"
#include "../ProjectWithOvertime.h"
#include "TestHelpers.h"

class LambdaTest : public testing::Test {
protected:
    std::unique_ptr<Lambda> l;
	std::unique_ptr<ProjectWithOvertime> p;

    void SetUp() override {
        p = std::make_unique<ProjectWithOvertime>("MiniBeispiel.DAT");
        l = std::make_unique<Lambda>(p->numJobs);
        l->order = p->topOrder;
    }

    void TearDown() override {
    }
};

