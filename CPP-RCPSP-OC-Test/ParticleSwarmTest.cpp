#include "TestHelpers.h"

using namespace std;

TEST(ParticleSwarmStaticTest, testDistance) {
	vector<int> al1 = { 0,1,2,3,4,5,6,7,8 };
	vector<int> al2 = { 0,1,3,4,2,7,6,5,8 };
	//ASSERT_EQ((0.0+0.0 + 2.0 + 1.0 + 1.0 + 2.0 + 0.0 + 2.0 + 0.0) / 9.0, ParticleSwarm::distance(al1, al2));
}

TEST(ParticleSwarmStaticTest, testSortDescendingFitness) {
}

TEST(ParticleSwarmStaticTest, testPathRelinking) {
	
}

