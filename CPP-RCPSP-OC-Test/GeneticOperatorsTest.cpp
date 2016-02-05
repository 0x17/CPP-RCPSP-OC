//
// Created by André Schnabel on 03.02.16.
//

#include <gtest/gtest.h>
#include "../GeneticAlgorithms/GeneticOperators.h"
#include "TestHelpers.h"

using namespace GeneticOperators;

TEST(GeneticOperatorsTest, testOnePointCrossover) {
    vector<int> mother, father, daughter(5, -1), son(5, -1);
	CrossoverData<vector<int>> orderDaughter = { mother, father, daughter };
	CrossoverData<vector<int>> orderSon = { father, mother, son };

    mother = { 0, 4, 2, 1, 3 };
    father = { 0, 1, 2, 3, 4 };
	onePointCrossover(orderDaughter, 2);
	vector<int> expDaughter = { 0, 4, 2, 1, 3 };	
	TestHelpers::arrayEquals(expDaughter, daughter);

	onePointCrossover(orderSon, 1);
	vector<int> expSon = { 0, 1, 4, 2, 3 };
	TestHelpers::arrayEquals(expSon, son);

	// Test crossover identical lists
	mother = { 0, 1, 4, 3, 2 };
	father = { 0, 1, 4, 3, 2 };
	expDaughter = { 0, 1, 4, 3, 2 };
	for(int q = 0; q < mother.size(); q++) {
		onePointCrossover(orderDaughter, q);
		TestHelpers::arrayEquals(expDaughter, daughter);
	}
}

TEST(GeneticOperatorsTest, testTwoPointCrossover) {
	vector<int> mother, father, daughter(5, -1), son(5, -1);
	CrossoverData<vector<int>> orderDaughter = { mother, father, daughter };
	CrossoverData<vector<int>> orderSon = { father, mother, son };

	mother = { 0, 4, 2, 1, 3 };
	father = { 0, 1, 2, 3, 4 };
	twoPointCrossover(orderDaughter, 1, 3);
	vector<int> expDaughter = { 0, 4, 1, 2, 3 };
	TestHelpers::arrayEquals(expDaughter, daughter);

	twoPointCrossover(orderSon, 1, 3);
	vector<int> expSon = { 0, 1, 4, 2, 3 };
	TestHelpers::arrayEquals(expSon, son);
}

TEST(GeneticOperatorsTest, testOnePointCrossoverAssociated) {
	vector<int> mother, father, daughter(5, -1), son(5, -1);
	vector<bool> motherB, fatherB, daughterB(5, false), sonB(5, false);

	CrossoverData<vector<int>> dataDaughter = { mother, father, daughter };
	CrossoverData<vector<bool>> dataDaughterB = { motherB, fatherB , daughterB };
	CrossoverData<vector<int>> dataSon = { father, mother, son };
	CrossoverData<vector<bool>> dataSonB = { fatherB, motherB, sonB };

	mother = { 0, 4, 2, 1, 3 };
	motherB = { false, true, false, true, false };
	father = { 0, 1, 2, 3, 4 };
	fatherB = { true, false, true, false, true };

	onePointCrossoverAssociated(dataDaughter, dataDaughterB, 2);
	vector<int> expDaughter = { 0, 4, 2, 1, 3 };
	vector<bool> expDaughterB = { false, false, false, false, false };
	TestHelpers::arrayEquals(expDaughter, daughter);
	TestHelpers::arrayEquals(expDaughterB, daughterB);

	onePointCrossoverAssociated(dataSon, dataSonB, 1);
	vector<int> expSon = { 0, 1, 4, 2, 3 };
	vector<bool> expSonB = { true, false, false, true, false };
	TestHelpers::arrayEquals(expSon, son);
	TestHelpers::arrayEquals(expSonB, sonB);
}