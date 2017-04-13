//
// Created by André Schnabel on 02.02.16.
//

#pragma once

#include <gtest/gtest.h>
#include "../ProjectWithOvertime.h"

class ProjectWithOvertimeTest : public testing::Test {
protected:
	std::unique_ptr<ProjectWithOvertime> p;

    void SetUp() override {
		p = std::make_unique<ProjectWithOvertime>("MiniBeispiel.DAT");
    }

    void TearDown() override {
    }

};
