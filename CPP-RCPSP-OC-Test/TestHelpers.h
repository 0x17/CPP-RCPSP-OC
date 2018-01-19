//
// Created by André Schnabel on 28.01.16.
//

#pragma once

#include <vector>
#include <list>

#include <gtest/gtest.h>

#include "../Matrix.h"

class TestHelpers {
public:
	static void floatArrayEquals(const std::vector<float> &expected, const std::vector<float> &actual) {
		ASSERT_EQ(expected.size(), actual.size());
		for (int i = 0; i<actual.size(); i++) {
			ASSERT_FLOAT_EQ(expected[i], actual[i])
				<< "expected[" << i << "]=" << expected[i]
				<< " != actual[" << i << "] = " << actual[i] << "!";
		}
	}

    template<class T>
    static void arrayEquals(const std::vector<T> &expected, const std::vector<T> &actual) {
        ASSERT_EQ(expected.size(), actual.size());
        for(int i=0; i<actual.size(); i++) {
			ASSERT_EQ(expected[i], actual[i])
				<< "expected[" << i << "]=" << expected[i]
				<< " != actual[" << i << "] = " << actual[i] << "!";
        }
    }

    template<class T>
    static void listEquals(const std::list<T> &expected, const std::list<T> &actual) {
        ASSERT_EQ(expected.size(), actual.size());
		std::equal(expected.begin(), expected.end(), actual.begin(), actual.end());
    }

    template<class T>
    static void matrixEquals(const Matrix<T> &expected, const Matrix<T> &actual) {
        ASSERT_EQ(expected.getM(), actual.getM());
        ASSERT_EQ(expected.getN(), actual.getN());
		for (int i = 0; i < actual.getM(); i++)
			for (int j = 0; j < actual.getN(); j++)
				ASSERT_EQ(expected(i, j), actual(i, j)) << "i=" << i << ",j=" << j << std::endl;
    }
};
