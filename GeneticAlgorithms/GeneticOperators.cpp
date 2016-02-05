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

void GeneticOperators::twoPointCrossover(CrossoverData<std::vector<int>> order, int q1, int q2) {
	int len = static_cast<int>(order.mother.size());

	// take first 0..q1 from mother
	for (int i = 0; i <= q1; i++) { order.daughter[i] = order.mother[i]; }

	// take q1+1..q2 from father (not in daughter before yet)
	for (int i = 0, ctr = q1 + 1; i < len, ctr <= q2; i++) {
		if (!Utils::rangeInclContains(order.daughter, 0, q1, order.father[i])) {
			order.daughter[ctr] = order.father[i];
			ctr++;
		}
	}

	// take remaining q2+1..len from mother again (not in daughter before yet)
	for (int i = q1 + 1, ctr = q2 + 1; i < len, ctr < len; i++ ) {
		if(!Utils::rangeInclContains(order.daughter, q1+1, q2, order.mother[i])) {
			order.daughter[ctr] = order.mother[i];
			ctr++;
		}
	}
}
