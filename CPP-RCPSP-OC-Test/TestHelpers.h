//
// Created by André Schnabel on 28.01.16.
//

#ifndef CPP_RCPSP_OC_TESTHELPERS_H
#define CPP_RCPSP_OC_TESTHELPERS_H

#include <vector>
#include <list>
#include <gtest/gtest.h>
#include "../Matrix.h"

class TestHelpers {
public:
    template<class T>
    static void arrayEquals(std::vector<T> &expected, std::vector<T> &actual) {
        ASSERT_EQ(expected.size(), actual.size());
        for(int i=0; i<actual.size(); i++) {
			ASSERT_EQ(expected[i], actual[i])
				<< "expected[" << i << "]=" << expected[i]
				<< " != actual[" << i << "] = " << actual[i] << "!";
        }
    }

    template<class T>
    static void listEquals(std::list<T> &expected, std::list<T> &actual) {
        ASSERT_EQ(expected.size(), actual.size());
		equal(expected.begin(), expected.end(), actual.begin(), actual.end());
    }

    template<class T>
    static void matrixEquals(Matrix<T> &expected, Matrix<T> &actual) {
        ASSERT_EQ(expected.getM(), actual.getM());
        ASSERT_EQ(expected.getN(), actual.getN());
        for(int i=0; i<actual.getM(); i++)
            for(int j=0; j<actual.getN(); j++)
                ASSERT_EQ(expected(i, j), actual(i, j));
    }
};


#endif //CPP_RCPSP_OC_TESTHELPERS_H
