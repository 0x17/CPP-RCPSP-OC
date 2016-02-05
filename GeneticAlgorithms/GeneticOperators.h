#ifndef __GENETIC_OPERATORS_H__
#define __GENETIC_OPERATORS_H__

#include <vector>

#include "../Utils.h"
#include "../Matrix.h"

#define SWAP_COMMON(code) \
	for(int i=1; i<order.size(); i++) \
	if(Utils::randRangeIncl(1, 100) <= pmutate && !adjMx(order[i - 1], order[i])) { code; }

#define OPC_COMMON(assignFromMother, assignFromFather, q) \
	for(int i = 0; i <= q; i++) { assignFromMother; } \
	for(int i = 0, ctr = q + 1; i<order.father.size(); i++) { \
		if(!Utils::rangeInclContains(order.daughter, 0, q, order.father[i])) { \
			assignFromFather; \
			ctr++; \
		} \
	}

namespace GeneticOperators
{
	void neighborhoodSwap(Matrix<char> &adjMx, std::vector<int> &order, int pmutate);
	template<class T>
	void neighborhoodSwapAssociated(Matrix<char> &adjMx, std::vector<int> &order, std::vector<T> &associated, int pmutate);

	template<class T>
	struct CrossoverData {
		T &mother, &father, &daughter;
	};

    void randomOnePointCrossover(CrossoverData<std::vector<int>> order);
	void onePointCrossover(CrossoverData<std::vector<int>> order, int q);
	void twoPointCrossover(CrossoverData<std::vector<int>> order, int q1, int q2);

    template<class U>
    void randomOnePointCrossoverAssociated(CrossoverData<std::vector<int>> order, CrossoverData<U> associated);
	template<class U>
	void onePointCrossoverAssociated(CrossoverData<std::vector<int>> order, CrossoverData<U> associated, int q);

	template<class T>
	inline void swap(std::vector<T> &order, int i1, int i2);
};

template<class T>
void GeneticOperators::swap(std::vector<T> &order, int i1, int i2) {
    T tmp = order[i1];
    order[i1] = order[i2];
    order[i2] = tmp;
}

template<class U>
void GeneticOperators::randomOnePointCrossoverAssociated(CrossoverData<std::vector<int>> order, CrossoverData<U> associated) {
    int q = Utils::randRangeIncl(0, static_cast<int>(order.mother.size()) - 1);
    onePointCrossoverAssociated(order, associated, q);
};

template <class U>
void GeneticOperators::onePointCrossoverAssociated(CrossoverData<std::vector<int>> order, CrossoverData<U> associated, int q) {
	OPC_COMMON(order.daughter[i] = order.mother[i]; associated.daughter[i] = associated.mother[i],
               order.daughter[ctr] = order.father[i]; associated.daughter[i] = associated.father[i],
               q)
}

template<class T>
void GeneticOperators::neighborhoodSwapAssociated(Matrix<char> &adjMx, std::vector<int> &order, std::vector<T> &associated, int pmutate) {
	SWAP_COMMON(swap(order, i - 1, i); swap(associated, i - 1, i))
}

#endif

