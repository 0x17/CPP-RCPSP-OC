
#include "MatrixTest.h"

TEST_F(MatrixTest, testToString) {
	ASSERT_EQ("Matrix(m=3,n=4,\n{{1,2,3,4},\n{5,6,7,8},\n{9,10,11,12}\n}\n", m->toString());
}