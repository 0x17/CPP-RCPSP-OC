//
// Created by André Schnabel on 28.01.16.
//

#ifndef CPP_RCPSP_OC_TESTHELPERS_H
#define CPP_RCPSP_OC_TESTHELPERS_H

#include <vector>
#include <gtest/gtest.h>

class TestHelpers {
public:
    template<class T>
    static void ArrayEquals(std::vector<T> &expected, std::vector<T> &actual) {
        ASSERT_EQ(expected.size(), actual.size());
        for(int i=0; i<actual.size(); i++) {
            ASSERT_EQ(expected[i], actual[i]);
        }
    }
};


#endif //CPP_RCPSP_OC_TESTHELPERS_H
