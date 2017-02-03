//
// Created by André Schnabel on 02.02.16.
//

#ifndef CPP_RCPSP_OC_PROJECTWITHOVERTIMETEST_H
#define CPP_RCPSP_OC_PROJECTWITHOVERTIMETEST_H

#include <gtest/gtest.h>
#include "../ProjectWithOvertime.h"

class ProjectWithOvertimeTest : public testing::Test {
protected:
	unique_ptr<ProjectWithOvertime> p;

    void SetUp() override {
		p = make_unique<ProjectWithOvertime>("MiniBeispiel.DAT");
    }

    void TearDown() override {
    }

};


#endif //CPP_RCPSP_OC_PROJECTWITHOVERTIMETEST_H
