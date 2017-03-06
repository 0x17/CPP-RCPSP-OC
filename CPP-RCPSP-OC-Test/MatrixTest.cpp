
#include "MatrixTest.h"
#include <array>

void assertEqualsExampleMatrix(Matrix<int> &m) {
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<4; j++) {
			ASSERT_EQ(i * 4 + j + 1, m.at(i, j));
		}
	}
}

TEST_F(MatrixTest, testNestedVectorConstructor) {
	assertEqualsExampleMatrix(*m);
}

TEST_F(MatrixTest, testLambdaConstructor) {
	Matrix<int> mx(3, 4, [](int i, int j) { return i * 4 + j + 1; });
	TestHelpers::matrixEquals(*m, mx);
}

TEST_F(MatrixTest, testCopyConstructor) {
	Matrix<int> mx(*m);
	assertEqualsExampleMatrix(mx);
}

TEST_F(MatrixTest, testRowColumnConstructor) {
	std::vector<int> vec = { 1, 2, 3, 4 };
	Matrix<int> mx(Matrix<int>::Mode::COLUMN_VECTOR, vec);
	for (int i = 0; i < 4; i++)
		ASSERT_EQ(vec[i], mx.at(i, 0));
	mx = Matrix<int>(Matrix<int>::Mode::ROW_VECTOR, vec);
	for (int i = 0; i < 4; i++)
		ASSERT_EQ(vec[i], mx.at(0, i));
}

TEST_F(MatrixTest, testDefaultConstructor) {
	Matrix<int> mx;
	ASSERT_EQ(0, mx.getM());
	ASSERT_EQ(0, mx.getN());
}

TEST_F(MatrixTest, testFixedSizeEmptyConstructor) {
	Matrix<int> mx(2, 3);
	ASSERT_EQ(2, mx.getM());
	ASSERT_EQ(3, mx.getN());
}

TEST_F(MatrixTest, testSingleValueConstructor) {
	Matrix<int> mx(2, 2, 23);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			ASSERT_EQ(23, mx(i, j));
}

TEST_F(MatrixTest, testGetMN) {
	ASSERT_EQ(3, m->getM());
	ASSERT_EQ(4, m->getN());
}

TEST_F(MatrixTest, testSubscriptOperator) {
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<4; j++) {
			ASSERT_EQ(i * 4 + j + 1, (*m)(i, j));
		}
	}
}

TEST_F(MatrixTest, testAssignOperator) {
	Matrix<int> mx = *m;
	assertEqualsExampleMatrix(mx);
}

TEST_F(MatrixTest, testResize) {
	m->resize(1, 1);
	ASSERT_EQ(1, m->getM());
	ASSERT_EQ(1, m->getN());
	ASSERT_EQ(1, m->at(0, 0));
}

TEST_F(MatrixTest, testRow) {
	std::vector<int> firstRow = { 1, 2, 3, 4 };
	std::vector<int> secondRow = { 5, 6, 7, 8 };
	std::vector<int> thirdRow = { 9, 10, 11, 12 };
	std::array<std::vector<int>, 3> expRows = { firstRow, secondRow, thirdRow };

	std::vector<int> actualRow;
	for(int i=0; i<3; i++) {
		actualRow = m->row(i);
		TestHelpers::arrayEquals(expRows[i], actualRow);
	}
}

TEST_F(MatrixTest, testColumn) {
	std::vector<int> firstCol = { 1, 5, 9 };
	std::vector<int> secondCol = { 2, 6, 10};
	std::vector<int> thirdCol = { 3, 7, 11 };
	std::vector<int> fourthCol = { 4, 8, 12 };
	std::array<std::vector<int>, 4> expColumns = { firstCol, secondCol, thirdCol, fourthCol };

	std::vector<int> actualColumn;
	for (int i = 0; i<4; i++) {
		actualColumn = m->column(i);
		TestHelpers::arrayEquals(expColumns[i], actualColumn);
	}
}

TEST_F(MatrixTest, testForeach) {
	Matrix<char> entryVisited(3, 4, 0);
	m->foreach([&entryVisited](int i, int j, int mij) { ASSERT_EQ(i * 4 + j + 1, mij); entryVisited(i, j) = 1; });
	for (int i = 0; i<3; i++)
		for (int j = 0; j<4; j++)
			ASSERT_EQ(1, entryVisited(i, j));
}

TEST_F(MatrixTest, testForeach2) {
	Matrix<char> entryVisited(3, 4, 0);
	m->foreach2([&entryVisited](int i, int j) {  entryVisited(i, j) = 1; });
	for(int i=0; i<3; i++)
		for(int j=0; j<4; j++)
			ASSERT_EQ(1, entryVisited(i,j));
}

TEST_F(MatrixTest, testForeachAssign) {
	Matrix<char> entryVisited(3, 4, 0);
	entryVisited.foreachAssign([](int i, int j) {  return 1; });
	for (int i = 0; i<3; i++)
		for (int j = 0; j<4; j++)
			ASSERT_EQ(1, entryVisited(i, j));
}

TEST_F(MatrixTest, testToString) {
	ASSERT_EQ("Matrix(m=3,n=4,\n{{1,2,3,4},\n{5,6,7,8},\n{9,10,11,12}\n}\n", m->toString());
}