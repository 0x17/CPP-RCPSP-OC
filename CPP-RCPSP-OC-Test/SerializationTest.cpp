//
// Created by André Schnabel on 13.04.17.
//

#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

#include "TestHelpers.h"
#include "../OnHold/Serialization.h"


//using namespace Serialization::JSON;
//using namespace Serialization::GAMS;

TEST(SerializationTest, testMatrixToJson) {
	/*Matrix<int> m({{1,2,3},{4,5,6},{7,8,9}});
	json11::Json obj = matrixToJson(m);
	ASSERT_EQ("[[1,2,3],[4,5,6],[7,8,9]]", boost::algorithm::erase_all_copy(obj.dump(), " "));*/
}

TEST(SerializationTest, testSerializeSet) {
	//ASSERT_EQ("set index /i0..i10/;\n", serializeSet("index", "i", 0, 10));
}