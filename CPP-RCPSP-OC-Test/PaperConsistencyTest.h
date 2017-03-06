#pragma once

#include "ProjectWithOvertimeTest.h"

class PaperConsistencyTest : public ProjectWithOvertimeTest {
protected:
	void SetUp() override;

	void setRevenueToCustomerA();
	void setRevenueToCustomerB();
	void setRevenueToCustomerC();
};

class MinimalProjectTest : public ProjectWithOvertimeTest {
protected:
	void SetUp() override;
};