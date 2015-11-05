//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_GENETICALGORITHMS_H
#define CPP_RCPSP_OC_GENETICALGORITHMS_H

#include <vector>
#include <algorithm>
#include <iostream>
#include "../ProjectWithOvertime.h"

using namespace std;

template<class Individual>
class GeneticAlgorithm {
public:
	virtual ~GeneticAlgorithm() {}

    pair<vector<int>, float> solve();

protected:
    ProjectWithOvertime &p;
    const int numGens, popSize, pmutate;

    GeneticAlgorithm(ProjectWithOvertime &_p) : p(_p), numGens(80), pmutate(5), popSize(100) {}

	void generateChildren(vector<pair<Individual, float>> & population);

    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;
	virtual vector<int> decode(Individual &i) = 0;

    void neighborhoodSwap(vector<int> &order);
    void onePointCrossover(vector<int> &motherOrder, vector<int> &fatherOrder, vector<int> &daughterOrder);
    void inline swap(vector<int> &order, int i1, int i2);

    pair<int, int> computePair(vector<bool> &alreadySelected);
};

template<class Individual>
pair<int, int> GeneticAlgorithm<Individual>::computePair(vector<bool> &alreadySelected) {
    pair<int, int> p;
    do {
        p.first = Utils::randRangeIncl(0, popSize-1);
    } while(alreadySelected[p.first]);
    do {
        p.second = Utils::randRangeIncl(0, popSize-1);
    } while(alreadySelected[p.second] || p.first == p.second);
    return p;
};

template<class Individual>
void GeneticAlgorithm<Individual>::generateChildren(vector<pair<Individual, float>> & pop) {
    vector<bool> alreadySelected(popSize);

    for(int childIx=popSize; childIx<popSize*2; childIx +=2) {
        pair<int, int> parentIndices = computePair(alreadySelected);
        crossover(pop[parentIndices.first].first, pop[parentIndices.second].first, pop[childIx].first);
        crossover(pop[parentIndices.second].first, pop[parentIndices.first].first, pop[childIx+1].first);
    }
}

template<class Individual>
pair<vector<int>, float> GeneticAlgorithm<Individual>::solve() {
    vector<pair<Individual, float>> pop(popSize*2);

    for(int i=0; i<popSize; i++) {
        pop[i].first = init(i);
        pop[i].second = -fitness(pop[i].first);
    }

	for(int i=popSize; i<popSize*2; i++) {
		pop[i].first = init(i);
		pop[i].second = 0.0f;
	}

    for(int i=0; i<numGens; i++) {
		cout << "Generation " << (i + 1) << endl;
        generateChildren(pop);

        for(int j=popSize; j<popSize*2; j++) {
            auto &indiv = pop[j].first;
            mutate(indiv);
            pop[j].second = -fitness(indiv);
        }

		sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });
		cout << "Obj=" << -pop[0].second << endl;
    }

    auto best = pop[0];
	return make_pair(decode(best.first), -best.second);
}

template<class Individual>
void GeneticAlgorithm<Individual>::swap(vector<int> &order, int i1, int i2) {
    int tmp = order[i1];
    order[i1] = order[i2];
    order[i2] = tmp;
}

template<class Individual>
void GeneticAlgorithm<Individual>::onePointCrossover(vector<int> &motherOrder, vector<int> &fatherOrder, vector<int> &daughterOrder) {
    int q = Utils::randRangeIncl(0, static_cast<int>(motherOrder.size())-1);

    for(int i=0; i<=q; i++)
		daughterOrder[i] = motherOrder[i];

    int ctr = q+1;
    for(int i=0; i<fatherOrder.size(); i++) {
        if(!Utils::rangeContains(motherOrder, 0, q, fatherOrder[i])) {
            daughterOrder[ctr] = fatherOrder[i];
            ctr++;
        }
    }
}

// FIXME: Contains bug making feasible orders infeasible!
template<class Individual>
void GeneticAlgorithm<Individual>::neighborhoodSwap(vector<int> &order) {
    for(int i=1; i<p.numJobs; i++)
		if(Utils::randRangeIncl(1, 100) <= pmutate && !p.adjMx[order[i - 1]][order[i]])
			swap(order, i - 1, i);
}

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
