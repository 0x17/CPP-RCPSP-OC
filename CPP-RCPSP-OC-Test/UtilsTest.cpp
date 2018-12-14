//
// Created by André Schnabel on 30.01.16.
//

#include "../Utils.h"
#include "TestHelpers.h"
#include <gtest/gtest.h>
#include <cstdio>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace boost::algorithm;
using namespace std;

const string
		testString = "These are some\nLines in a\nTextfile.",
		testFilename = "tmpfile";

void createAndWriteTestfile() {
	ofstream of(testFilename);
	of << testString;
	of.close();
}

TEST(UtilsTest, testSlurp) {
	createAndWriteTestfile();

	string contents = Utils::slurp(testFilename);
	ASSERT_EQ(testString, contents);

	boost::filesystem::remove(testFilename);
}

TEST(UtilsTest, testReadLinesSynthetic) {
	createAndWriteTestfile();
	vector<string> lines = Utils::readLines(testFilename);
	ASSERT_EQ(3, lines.size());
	ASSERT_EQ("These are some", lines[0]);
	ASSERT_EQ("Lines in a", lines[1]);
	ASSERT_EQ("Textfile.", lines[2]);
	boost::filesystem::remove(testFilename);
}

TEST(UtilsTest, testReadLines) {
    auto actLines = Utils::readLines("Data/MiniBeispiel.DAT");
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

TEST(UtilsTest, testRandBool) {
	int sum = 0;
	for(int i=0; i<100; i++) {
		sum += Utils::bool2int(Utils::randBool());
	}
	ASSERT_TRUE(sum <= 60);
	ASSERT_TRUE(sum >= 40);
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

TEST(UtilsTest, testRangeInclContains) {
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

TEST(UtilsTest, testIndexOfFirstEqualTo) {
	vector<int> nums = { 5, 2, 4, 15, 9, 15, 3, 3, 3 };
	ASSERT_EQ(3, Utils::indexOfFirstEqualTo(15, nums));
}

TEST(UtilsTest, testPickWithDistribution) {
	int q;
    vector<float> probs = { 1.0f, 0.0f, 0.0f };
	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(0, Utils::pickWithDistribution(probs));
	}

    probs = { 0.5f, 0.5f, 0.0f };
	for (int i = 0; i < 100; i++) {
		q = Utils::pickWithDistribution(probs);
		ASSERT_TRUE(q == 0 || q == 1);
	}

	probs = { 0.0f, 0.25f, 0.0f, 0.75f, 0.0f };
	for (int i = 0; i < 100; i++) {		
		q = Utils::pickWithDistribution(probs);
		ASSERT_TRUE(q == 1 || q == 3);
	}
	q = Utils::pickWithDistribution(probs, 0.0f);
	ASSERT_TRUE(q == 1);
	q = Utils::pickWithDistribution(probs, 1.0f);
	ASSERT_TRUE(q == 3);
	q = Utils::pickWithDistribution(probs, 0.75f);
	ASSERT_TRUE(q == 3);
	q = Utils::pickWithDistribution(probs, 0.1f);
	ASSERT_TRUE(q == 1);
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

TEST(UtilsTest, testSwap) {
	vector<int> nums = { 1, 2, 3, 4 };
	Utils::swap(nums, 1, 2);
	vector<int> expNums = { 1, 3, 2, 4 };
	TestHelpers::arrayEquals(expNums, nums);
}

TEST(UtilsTest, testMaxInRangeIncl) {
    ASSERT_EQ(0, Utils::maxInRangeIncl(0, 5, [](int x) { return static_cast<float>(-x); }));
    vector<float> nums = { 1.0f, 4.0f, 2.0f, 20.0f, 8.0f, 1.0f, 0.0f };
    ASSERT_EQ(20, Utils::maxInRangeIncl(0, static_cast<int>(nums.size())-1, [&](int i) { return nums[i]; }));
}

string removeWhitespace(string s) {
	return replace_all_copy(replace_all_copy(replace_all_copy(s, ".", ""), "\\", ""), "/", "");
}

TEST(UtilsTest, testFilenamesInDirWithExt) {
    auto fnames = Utils::filenamesInDirWithExt("Data", "DAT");
    list<string> expFnames = { "Data/MiniBeispiel.DAT", "Data/QBWLBeispiel.DAT" };
	auto expFnamesRplced = Utils::mapLst<string(string), string, string>(removeWhitespace, expFnames);
    TestHelpers::listEquals(expFnamesRplced, fnames);

	list<string> expFnamesCores = {
		"FixedDeadlineModels", "ListModel", "NaiveModels", "OvertimeBoundModels", "TimeWindowModels"
	};
	const string dirName = "LSModels";
	fnames = Utils::filenamesInDirWithExt(dirName, ".cpp");	
	ASSERT_EQ(5, fnames.size());
	for(string fname : fnames) {
		ASSERT_TRUE(boost::ends_with(fname, ".cpp"));
	}

}

TEST(UtilsTest, testFilenamesInDir) {
	string dirName = "LSModels";
	char sep = boost::filesystem::path::preferred_separator;
	auto fnames = Utils::filenamesInDir(dirName);
	ASSERT_EQ(10, fnames.size());
	list<string> expFnamesCores = {
		"FixedDeadlineModels", "ListModel", "NaiveModels", "OvertimeBoundModels", "TimeWindowModels"
	};
	for(string expCore : expFnamesCores) {
		ASSERT_TRUE(std::find(fnames.begin(), fnames.end(), dirName + sep + expCore + ".cpp") != fnames.end());
		ASSERT_TRUE(std::find(fnames.begin(), fnames.end(), dirName + sep + expCore + ".h") != fnames.end());
	}
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

TEST(UtilsTest, testConstructVector) {
	auto v = Utils::constructVector<int>(5, [](int i) { return i+1; });
	vector<int> ev = {1,2,3,4,5};
	ASSERT_EQ(ev, v);
}

TEST(UtilsTest, testConstructList) {
	auto l = Utils::constructList<int>(5, [](int i) { return i+1; });
	list<int> el = {1,2,3,4,5};
	ASSERT_EQ(el, l);
}

TEST(UtilsTest, testPartitionDirectory) {
	char sep = boost::filesystem::path::preferred_separator;
	string tmpDir = "TMP_DIR";

	boost::filesystem::create_directory(tmpDir);
	for(int ix=0; ix<40; ix++)
		Utils::spit("...", tmpDir + sep + "file"+to_string(ix+1)+".txt");

	Utils::partitionDirectory(tmpDir, 4, "_");

	vector<string> expDirectories = {
		"TMP_DIR_1",
		"TMP_DIR_2",
		"TMP_DIR_3",
		"TMP_DIR_4"
	};

	for(int i=0; i<4; i++) {
		/*for(int j=0; j<10; j++) {
			ASSERT_TRUE(boost::filesystem::exists(expDirectories[i] + sep + "file" + to_string(j+1) + ".txt"));
		}*/
		ASSERT_EQ(10, Utils::filenamesInDir(expDirectories[i]).size());
	}

	for (string expDir : expDirectories) {
		ASSERT_TRUE(boost::filesystem::exists(expDir));
	}

	boost::filesystem::remove_all(tmpDir);
	for(string expDir : expDirectories) {
		boost::filesystem::remove_all(expDir);
	}
}

TEST(UtilsTest, testDeserializeSchedule) {
	const string SCHEDULE_FN = "schedule.txt";
	Utils::spit("1->0\n2->0\n3->2\n4->5\n5->10\n6->10\n", SCHEDULE_FN);
	vector<int> sts = Utils::deserializeSchedule(6, SCHEDULE_FN);
	vector<int> expSts = { 0, 0, 2, 5, 10, 10 };
	TestHelpers::arrayEquals(expSts, sts);
	boost::filesystem::remove(SCHEDULE_FN);
}

TEST(UtilsTest, testSplitLines) {
	const string LINES = "First line\nSecond line\nThird line";
	vector<string> parts = Utils::splitLines(LINES);
	vector<string> expParts = { "First line", "Second line", "Third line" };
	TestHelpers::arrayEquals(expParts, parts);
}

TEST(UtilsTest, testIndexOf) {
	vector<int> nums = { 1, 3, 8, 10, 9, 5, 4 };
	ASSERT_EQ(3, Utils::indexOf(nums, [](int i) { return i % 10 == 0;  }));
}

TEST(UtilsTest, testSum) {
	vector<int> nums = Utils::constructVector<int>(100, [](int i) {return i+1;});
	int res = static_cast<int>(100.0f*101.0f/2.0f);
	Matrix<char> numMx(2, 50, [](int i, int j) { return i*50+j+1; });
	ASSERT_EQ(res, Utils::sum([](int i) {return i+1;}, 0, 100));
	ASSERT_EQ(res, Utils::sum(nums));
	ASSERT_EQ(res, Utils::sum(numMx));
}

TEST(UtilsTest, testTransitiveClosure) {
	{
		Matrix<char> inMx({
								  {0, 1, 0, 0},
								  {0, 0, 1, 0},
								  {0, 0, 0, 1},
								  {0, 0, 0, 0}
						  });
		Matrix<char> outMx({
								   {0, 1, 1, 1},
								   {0, 0, 1, 1},
								   {0, 0, 0, 1},
								   {0, 0, 0, 0}
						   });
		TestHelpers::matrixEquals(outMx, Utils::transitiveClosure(inMx));
	}
	{
		Matrix<char> inMx({{1, 1, 0, 1},
						   {0, 1, 1, 0},
						   {0, 0, 1, 1},
						   {0, 0, 0, 1}
						  });
		Matrix<char> outMx({
								   {1, 1, 1, 1},
								   {0, 1, 1, 1},
								   {0, 0, 1, 1},
								   {0, 0, 0, 1}
						   });
		TestHelpers::matrixEquals(outMx, Utils::transitiveClosure(inMx));
	}
}
