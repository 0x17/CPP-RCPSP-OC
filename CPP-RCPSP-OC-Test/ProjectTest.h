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

    virtual void SetUp() {
        p = new Project("MiniBeispiel.DAT");
    }

    virtual void TearDown() {
        delete p;
    }

};


#endif //CPP_RCPSP_OC_PROJECTTEST_H
