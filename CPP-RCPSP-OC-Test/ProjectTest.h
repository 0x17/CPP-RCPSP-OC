//
// Created by André Schnabel on 28.01.16.
//

#ifndef CPP_RCPSP_OC_PROJECTTEST_H
#define CPP_RCPSP_OC_PROJECTTEST_H

#include <gtest/gtest.h>
#include "../Project.h"

class ProjectTest : public testing::Test {
protected:
    Project *p;

	void SetUp() override {
        p = new Project("MiniBeispiel.DAT");
    }

	void TearDown() override {
        delete p;
    }

};


#endif //CPP_RCPSP_OC_PROJECTTEST_H
