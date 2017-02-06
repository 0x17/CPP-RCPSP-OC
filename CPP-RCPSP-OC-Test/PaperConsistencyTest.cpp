#include "PaperConsistencyTest.h"
#include "TestHelpers.h"
#include "../GeneticAlgorithms/TimeWindow.h"

const static string EXAMPLE_STR =
		"************************************************************************\n"
		"file with basedata            : ..\\EXAMPLE\\EXPL.BAS\n"
		"initial value random generator: 12164\n"
		"************************************************************************\n"
		"projects                      :  1\n"
		"jobs (incl. supersource/sink ):  8\n"
		"horizon                       :  13\n"
		"RESOURCES\n"
		"  - renewable                 :  1   R\n"
		"  - nonrenewable              :  0   N\n"
		"  - doubly constrained        :  0   D\n"
		"************************************************************************\n"
		"PROJECT INFORMATION:\n"
		"pronr.  #jobs rel.date duedate tardcost  MPM-Time\n"
		"    1     8       0       13        0       33\n"
		"************************************************************************\n"
		"PRECEDENCE RELATIONS:\n"
		"jobnr.    #modes  #successors   successors\n"
		"   1        1          2           2   3\n"
		"   2        1          1           4\n"
		"   3        1          1           5\n"
		"   4        1          1           6\n"
		"   5        1          1           7\n"
		"   6        1          1           7\n"
		"   7        1          1           8\n"
		"************************************************************************\n"
		"REQUESTS/DURATIONS:\n"
		"jobnr. mode duration  R 1\n"
		"------------------------------------------------------------------------\n"
		"  1      1     0       0\n"
		"  2      1     3       3\n"
		"  3      1     2       2\n"
		"  4      1     2       2\n"
		"  5      1     3       1\n"
		"  6      1     1       2\n"
		"  7      1     2       1\n"
		"  8      1     0       0\n"
		"************************************************************************\n"
		"RESOURCEAVAILABILITIES:\n"
		"  R 1\n"
		"    4\n"
		"************************************************************************\n";

void PaperConsistencyTest::SetUp() {
	p = make_unique<ProjectWithOvertime>("ExampleFromPaper", EXAMPLE_STR);
	p->kappa = { 10 };
}

void PaperConsistencyTest::setRevenueToCustomerA() {
	p->eachPeriod([&](int t) {
		 p->revenue[t] = (t <= 11) ? 10.0f : 0.0f;
	});
}

void PaperConsistencyTest::setRevenueToCustomerB() {
	p->eachPeriod([&](int t) {
		if (t < 8) p->revenue[t] = 60.0f;
		else if (t == 8) p->revenue[t] = 45.0f;
		else if (t == 9) p->revenue[t] = 30.0f;
		else if (t == 10) p->revenue[t] = 15.0f;
		else p->revenue[t] = 0.0f;
	});
}

void PaperConsistencyTest::setRevenueToCustomerC() {
	p->eachPeriod([&](int t) {
		if (t <= 8) p->revenue[t] = 20.0f;
		else if (t == 9) p->revenue[t] = 15.0f;
		else p->revenue[t] = 0.0f;
	});
}

TEST_F(PaperConsistencyTest, testProjectCorrectlyLoaded) {
	ASSERT_EQ(8, p->numJobs);
	ASSERT_EQ(1, p->numRes);
	ASSERT_TRUE(p->numPeriods > 13);

	vector<int> durations = { 0, 3, 2, 2, 3, 1, 2, 0 };
	TestHelpers::arrayEquals(durations, p->durations);

	Matrix<int> demands(Matrix<int>::Mode::COLUMN_VECTOR, { 0, 3, 2, 2, 1, 2, 1, 0 });
	TestHelpers::matrixEquals(demands, p->demands);

	Matrix<char> adjMx({
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0}
	});
	TestHelpers::matrixEquals(adjMx, p->adjMx);

	ASSERT_EQ(4, p->capacities[0]);
	ASSERT_EQ(10, p->kappa[0]);
	ASSERT_EQ(2, p->zmax[0]);
}

TEST_F(PaperConsistencyTest, testRevenuesOfCustomers) {
	setRevenueToCustomerA();
	ASSERT_EQ(0.0f, p->revenue[12]);
	ASSERT_EQ(10.0f, p->revenue[0]);
	ASSERT_EQ(10.0f, p->revenue[9]);
	setRevenueToCustomerB();
	ASSERT_EQ(0.0f, p->revenue[12]);
	ASSERT_EQ(60.0f, p->revenue[0]);
	ASSERT_EQ(30.0f, p->revenue[9]);
	setRevenueToCustomerC();
	ASSERT_EQ(0.0f, p->revenue[12]);
	ASSERT_EQ(20.0f, p->revenue[0]);
	ASSERT_EQ(15.0f, p->revenue[9]);
}

static vector<int> scheduleFigure1 = { 0, 0, 3, 3, 5, 5, 8, 10 };
static vector<int> scheduleFigure2 = { 0, 0, 0, 3, 3, 5, 6, 8 };
static vector<int> scheduleFigure3 = { 0, 0, 2, 3, 4, 5, 7, 9 };

TEST_F(PaperConsistencyTest, testExampleScheduleFeasibility) {
	ASSERT_TRUE(p->isScheduleFeasible(scheduleFigure1));
	ASSERT_TRUE(p->isScheduleFeasible(scheduleFigure2));
	ASSERT_TRUE(p->isScheduleFeasible(scheduleFigure3));
}

TEST_F(PaperConsistencyTest, testExampleScheduleCosts) {
	ASSERT_EQ(0, p->totalCosts(scheduleFigure1));
	ASSERT_EQ(20, p->totalCosts(scheduleFigure2));
	ASSERT_EQ(10, p->totalCosts(scheduleFigure3));
}

TEST_F(PaperConsistencyTest, testLambdaCrossovers) {
	Lambda mother({1,2,3,4,5,6});
	Lambda father({1,3,5,2,4,6});
	Lambda expDaughter({1,2,3,5,4,6});
	Lambda daughter(6);
	daughter.onePointCrossover(mother, father, 2);
	ASSERT_EQ(expDaughter.order, daughter.order);
}

TEST_F(PaperConsistencyTest, testLambdaBetaCrossover) {
	LambdaBeta mother({1,2,3,4,5,6}, {1,1,0,0,0,0});
	LambdaBeta father({1,3,5,2,4,6}, {1,0,1,0,1,0});
	LambdaBeta expDaughter({1,2,3,5,4,6},{1,1,0,1,1,0});
	LambdaBeta daughter(6);
	daughter.onePointCrossover(mother, father, 2);
	ASSERT_EQ(expDaughter.order, daughter.order);
	ASSERT_EQ(expDaughter.beta, daughter.beta);
}

TEST_F(PaperConsistencyTest, testLambdaZrCrossover) {
}

TEST_F(PaperConsistencyTest, testLambdaZrtCrossover) {
}
