//
// Created by André Schnabel on 03.02.16.
//

#include <gtest/gtest.h>
#include "../GeneticAlgorithms/GeneticOperators.h"

TEST(GeneticOperatorsTest, testOnePointCrossover) {
    vector<int> mother, father, daughter(5, -1);
    mother = { 0, 1, 2, 3, 4 };
    father = { 0, 1, 2, 3, 4 };
    GeneticOperators::CrossoverData<vector<int>> order = { mother, father, daughter };
    GeneticOperators::onePointCrossover(order, 2);
}