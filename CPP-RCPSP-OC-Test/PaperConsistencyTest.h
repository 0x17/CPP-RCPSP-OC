#pragma once

#include <gtest/gtest.h>
#include "ProjectWithOvertimeTest.h"

class PaperConsistencyTest : public ProjectWithOvertimeTest {
protected:
	void SetUp() override;

	void setRevenueToCustomerA();
	void setRevenueToCustomerB();
	void setRevenueToCustomerC();
};