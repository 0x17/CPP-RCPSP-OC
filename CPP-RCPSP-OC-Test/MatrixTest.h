#pragma once

#include <gtest/gtest.h>
#include "TestHelpers.h"
#include "../Matrix.h"
#include <memory>

class MatrixTest : public testing::Test {
protected:
	std::unique_ptr<Matrix<int>> m;

	void SetUp() override {
		m = std::unique_ptr<Matrix<int>>(new Matrix<int>({
			{1,2,3,4},
			{5,6,7,8},
			{9,10,11,12}
		}));
	}
};
