//
// Created by André Schnabel on 21.12.17.
//

#include <gtest/gtest.h>
#include "../Libraries/json11.hpp"
#include "../JsonUtils.h"
#include "../Matrix.h"

using namespace std;
using namespace json11;

TEST(UtilsTest, testMergeObjects) {
    auto a = Json::object { {"key1", 1} };
    auto b = Json::object { {"key2", 2} };
    auto c = JsonUtils::mergeObjects(a, b);
    std::string cs = c.dump();
    ASSERT_TRUE(c["key1"].is_number());
    ASSERT_TRUE(c["key2"].is_number());
    ASSERT_EQ(1, c["key1"].number_value());
    ASSERT_EQ(2, c["key2"].number_value());
}

TEST(UtilsTest, testFillIntFieldsWithObject) {
    int field1, field2;
    field1 = field2 = 0;
    Json obj = Json::object { {"field1", 10}, {"field2", 20} };
    JsonUtils::fillIntFieldsWithObject(obj, {"field1", "field2"}, {&field1, &field2});
    ASSERT_EQ(10, field1);
    ASSERT_EQ(20, field2);
}

TEST(UtilsTest, testFillStrFieldsWithObject) {
    string field1, field2;
    field1 = field2 = "";
    Json obj = Json::object { {"field1", "10"}, {"field2", "20"} };
    JsonUtils::fillStrFieldsWithObject(obj, {"field1", "field2"}, {&field1, &field2});
    ASSERT_EQ("10", field1);
    ASSERT_EQ("20", field2);
}

TEST(UtilsTest, testMatrixIntToJson) {
    Matrix<int> mx(2, 2, [](int i, int j) {
        return i*2+j;
    });
    auto actualArray = JsonUtils::matrixIntToJson(mx);
    Json::array expectedArray = {
            Json::array { 0, 1 },
            Json::array { 2, 3 }
    };
    ASSERT_EQ(2, actualArray.size());
    ASSERT_EQ(2, actualArray[0].array_items().size());
    ASSERT_EQ(expectedArray, actualArray);
}

TEST(UtilsTest, testMatrixIntFromJson) {
    Json obj = Json::array { Json::array {0,1}, Json::array {2,3} };
    Matrix<int> expectedMatrix({{0,1}, {2,3}});
    auto actualMatrix = JsonUtils::matrixIntFromJson(obj);
    ASSERT_EQ(expectedMatrix, actualMatrix);
}