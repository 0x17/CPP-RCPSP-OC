//
// Created by André Schnabel on 03.02.16.
//

#include "GeneticOperators.h"

void GeneticOperators::neighborhoodSwap(Matrix<char> &adjMx, std::vector<int> &order, int pmutate) {
    SWAP_COMMON(swap(order, i - 1, i))
}

void GeneticOperators::randomOnePointCrossover(CrossoverData<std::vector<int>> order) {
    int q = Utils::randRangeIncl(0, static_cast<int>(order.mother.size()) - 1);
    onePointCrossover(order, q);
}

void GeneticOperators::onePointCrossover(CrossoverData<std::vector<int>> order, int q) {
    OPC_COMMON(order.daughter[i] = order.mother[i],
               order.daughter[ctr] = order.father[i],
               q)
}
