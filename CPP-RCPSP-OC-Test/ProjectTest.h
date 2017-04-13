//
// Created by André Schnabel on 28.01.16.
//

#pragma once

#include <gtest/gtest.h>
#include "../Project.h"

class ProjectTest : public testing::Test {
protected:
    std::unique_ptr<Project> p;

	void SetUp() override {
        p = std::make_unique<Project>("MiniBeispiel.DAT");
    }

	void TearDown() override {
    }

};
