//
// Created by André Schnabel on 28.01.16.
//

#ifndef CPP_RCPSP_OC_PROJECTTEST_H
#define CPP_RCPSP_OC_PROJECTTEST_H

#include <gtest/gtest.h>
#include "../Project.h"

class ProjectTest : public testing::Test {
protected:
    std::unique_ptr<Project> p;

	void SetUp() override {
        p = make_unique<Project>("MiniBeispiel.DAT");
    }

	void TearDown() override {
    }

};


#endif //CPP_RCPSP_OC_PROJECTTEST_H
