//
// Created by André Schnabel on 30.01.16.
//

#include "UtilsTest.h"
#include "../Utils.h"
#include "TestHelpers.h"
#include <cstdio>
#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;

TEST(UtilsTest, testReadLines) {
    auto actLines = Utils::readLines("MiniBeispiel.DAT");
	auto actLinesTr = Utils::mapVec<string(string), string, string>([](string line) { return trim_copy(line); }, actLines);
    ASSERT_EQ("************************************************************************", actLinesTr[0]);
    ASSERT_EQ("projects                      :  1", actLinesTr[4]);
    ASSERT_EQ("1->1", actLinesTr[actLinesTr.size()-2]);
}

TEST(UtilsTest, testExtractIntFromStr) {
    string s = "jobs (incl. supersource/sink ):  5";
    int n = Utils::extractIntFromStr(s, "jobs \\(incl. supersource\\/sink \\):  (\\d+)");
    ASSERT_EQ(5, n);
}

TEST(UtilsTest, testExtractIntsFromLine) {
    auto actualInts = Utils::extractIntsFromLine("   1        1          2           2   4");
    vector<int> expInts = { 1, 1, 2, 2, 4 };
    TestHelpers::arrayEquals(expInts, actualInts);
}

TEST(UtilsTest, testBatchResize) {
    vector<int> v1, v2, v3;
    Utils::batchResize(10, {&v1, &v2, &v3});
    ASSERT_EQ(10, v1.size());
    ASSERT_EQ(10, v2.size());
    ASSERT_EQ(10, v3.size());
}

TEST(UtilsTest, testMaxBinary) {
    ASSERT_EQ(10, Utils::max(-1, 10));
    ASSERT_EQ(10, Utils::max(0, 10));
    ASSERT_EQ(10, Utils::max(10, 10));
    ASSERT_EQ(-10, Utils::max(-20, -10));
}

TEST(UtilsTest, testMinBinary) {
    ASSERT_EQ(-1, Utils::min(-1, 10));
    ASSERT_EQ(0, Utils::min(0, 10));
    ASSERT_EQ(10, Utils::min(10, 10));
    ASSERT_EQ(-20, Utils::min(-20, -10));
}

TEST(UtilsTest, testMaxTernary) {
    ASSERT_EQ(10, Utils::max(1, 2, 10));
    ASSERT_EQ(-1, Utils::max(-50, -40, -1));
}

TEST(UtilsTest, testMinTernary) {
    ASSERT_EQ(10, Utils::min(10, 20, 100));
    ASSERT_EQ(-1, Utils::min(100, 10, -1));
}

TEST(UtilsTest, testSerializeSchedule) {
    vector<int> sts = { 0, 1, 2, 3, 4, 5 };
    vector<string> expLines = {
        "1->0",
        "2->1",
        "3->2",
        "4->3",
        "5->4",
        "6->5",
    };
    string tmpFile = "TMP_SCHEDULE.txt";
    Utils::serializeSchedule(sts, tmpFile);
    auto actLines = Utils::readLines(tmpFile);
    TestHelpers::arrayEquals(expLines, actLines);
    remove(tmpFile.c_str());
}

TEST(UtilsTest, testSerializeProfit) {
    float profit = 3.141f;
    string tmpFile = "TMP_PROFIT.txt";
    Utils::serializeProfit(profit, tmpFile);
    auto lines = Utils::readLines(tmpFile);
    ASSERT_EQ("3.141", lines[0].substr(0, 5));
    remove(tmpFile.c_str());

}

TEST(UtilsTest, testRandomRangeIncl) {
    ASSERT_EQ(0, Utils::randRangeIncl(0, 0));
    ASSERT_EQ(1, Utils::randRangeIncl(1, 1));
    int q = Utils::randRangeIncl(0, 1);
    ASSERT_TRUE(q == 0 || q == 1);
    q = Utils::randRangeIncl(0, 9);
    ASSERT_TRUE(q <= 9);
    ASSERT_TRUE(q >= 0);
}

TEST(UtilsTest, testRandUnitFloat) {
    float q = Utils::randUnitFloat();
    ASSERT_TRUE(q >= 0.0f && q <= 1.0f);
}

TEST(UtilsTest, testRangeContains) {
    vector<int> nums = { 1, 4, 2, 8, 12, 16 };
    ASSERT_TRUE(Utils::rangeInclContains(nums, 0, 0, 1));
    ASSERT_FALSE(Utils::rangeInclContains(nums, 0, 0, 4));
    ASSERT_TRUE(Utils::rangeInclContains(nums, 0, 5, 16));
    ASSERT_TRUE(Utils::rangeInclContains(nums, 5, 5, 16));
    ASSERT_FALSE(Utils::rangeInclContains(nums, 0, 5, 15));
}

TEST(UtilsTest, testIndexOfNthEqualTo) {
    vector<int> nums = { 5, 2, 4, 15, 9, 15, 3, 3, 3 };
    ASSERT_EQ(3, Utils::indexOfNthEqualTo(0, 15, nums));
    ASSERT_EQ(5, Utils::indexOfNthEqualTo(1, 15, nums));
    ASSERT_EQ(0, Utils::indexOfNthEqualTo(0, 5, nums));
    ASSERT_EQ(6, Utils::indexOfNthEqualTo(0, 3, nums));
    ASSERT_EQ(7, Utils::indexOfNthEqualTo(1, 3, nums));
    ASSERT_EQ(8, Utils::indexOfNthEqualTo(2, 3, nums));
}

TEST(UtilsTest, testPickWithDistribution) {
    vector<float> probs = { 1.0f, 0.0f, 0.0f };
    for(int i=0; i<10; i++)
        ASSERT_EQ(0, Utils::pickWithDistribution(probs));

    probs = { 0.5f, 0.5f, 0.0f };
    int q = Utils::pickWithDistribution(probs);
    ASSERT_TRUE(q == 0 || q == 1);
}

TEST(UtilsTest, testSpit) {
    string tmpFile = "SPIT_FILE.txt";
    Utils::spit("Are you mad now?\nHow about now?", tmpFile);
    auto lines = Utils::readLines(tmpFile);
    ASSERT_EQ(2, lines.size());
    ASSERT_EQ("Are you mad now?", lines[0]);
    ASSERT_EQ("How about now?", lines[1]);
    remove(tmpFile.c_str());
}

TEST(UtilsTest, testSpitAppend) {
    string tmpFile = "SPIT_APPEND_FILE.txt";
    Utils::spit("Are you mad now?\nHow about now?\n", tmpFile);
    Utils::spitAppend("Well how about now?", tmpFile);
    auto lines = Utils::readLines(tmpFile);
    ASSERT_EQ(3, lines.size());
    ASSERT_EQ("Are you mad now?", lines[0]);
    ASSERT_EQ("How about now?", lines[1]);
    ASSERT_EQ("Well how about now?", lines[2]);
    remove(tmpFile.c_str());
}

TEST(UtilsTest, testMaxInRangeIncl) {
    ASSERT_EQ(0, Utils::maxInRangeIncl(0, 5, [](int x) { return static_cast<float>(-x); }));
    vector<float> nums = { 1.0f, 4.0f, 2.0f, 20.0f, 8.0f, 1.0f, 0.0f };
    ASSERT_EQ(20, Utils::maxInRangeIncl(0, nums.size()-1, [&](int i) { return nums[i]; }));
}

string removeWhitespace(string s) {
	return replace_all_copy(replace_all_copy(replace_all_copy(s, ".", ""), "\\", ""), "/", "");
}

TEST(UtilsTest, testFilenamesInDirWithExt) {
    auto fnames = Utils::filenamesInDirWithExt(".", "DAT");
    list<string> expFnames = { "MiniBeispiel.DAT", "QBWLBeispiel.DAT" };
	auto expFnamesRplced = Utils::mapLst<string(string), string, string>(removeWhitespace, expFnames);
    TestHelpers::listEquals(expFnamesRplced, fnames);
}

TEST(UtilsTest, testMapVec) {
	vector<int> nums = { 1, 2, 4, 8, 16 };
	vector<int> doubled = { 2, 4, 8, 16, 32 };
	auto actualApplied = Utils::mapVec<int(int), int, int>([](int x) { return x * 2; }, nums);
	TestHelpers::arrayEquals(doubled, actualApplied);
	ASSERT_EQ(1, nums[0]);
}

TEST(UtilsTest, testMapLst) {
	list<int> nums = { 1, 2, 4, 8, 16 };
	list <int> doubled = { 2, 4, 8, 16, 32 };
	auto actualApplied = Utils::mapLst<int(int), int, int>([](int x) { return x * 2; }, nums);
	TestHelpers::listEquals(doubled, actualApplied);
	ASSERT_EQ(1, nums.front());
}