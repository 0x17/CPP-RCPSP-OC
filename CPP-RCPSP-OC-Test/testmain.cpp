#include <gtest/gtest.h>

TEST(MeinTestFall, MeinTest) {
	ASSERT_EQ(4, 4);
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();	
}