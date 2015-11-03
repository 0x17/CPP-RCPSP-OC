//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_GENETICALGORITHMS_H
#define CPP_RCPSP_OC_GENETICALGORITHMS_H

#include <vector>
#include "../ProjectWithOvertime.h"

using namespace std;

template<class Individual>
class GeneticAlgorithm {
public:
	virtual ~GeneticAlgorithm() {}

    void generateChildren(vector<Individual> &population);
    pair<float, Individual> solve();

protected:
    ProjectWithOvertime &p;
    const int numGens, popSize, pmutate;

    GeneticAlgorithm(ProjectWithOvertime &_p) : p(_p), numGens(80), pmutate(5), popSize(100) {}
    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;

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
void GeneticAlgorithm<Individual>::generateChildren(vector<Individual> &population) {
    vector<bool> alreadySelected;

    for(int childIx =popSize; childIx <popSize*2; childIx +=2) {
        pair<int, int> parentIndices = computePair(alreadySelected);
        crossover(population[parentIndices.first], population[parentIndices.second], population[childIx]);
        crossover(population[parentIndices.second], population[parentIndices.first], population[childIx+1]);
    }
}

template<class Individual>
pair<float, Individual> GeneticAlgorithm<Individual>::solve() {
    vector<pair<float, Individual>> pop(popSize*2);

    for(int i=0; i<popSize; i++) {
        pop[i].second = init(i);
        pop[i].first = -fitness(pop[i].second);
    }

    for(int i=0; i<numGens; i++) {
        generateChildren(pop);

        for(int j=popSize; j<popSize*2; j++) {
            auto indiv = pop[j].second;
            mutate(indiv);
            pop[j].first = -fitness(indiv);
        }

        sort(pop.begin(), pop.end());
    }

    return pop[0];
}

template<class Individual>
void inline GeneticAlgorithm<Individual>::swap(vector<int> &order, int i1, int i2) {
    int tmp = order[i1];
    order[i1] = order[i2];
    order[i2] = tmp;
}

template<class Individual>
void GeneticAlgorithm<Individual>::onePointCrossover(vector<int> &motherOrder, vector<int> &fatherOrder, vector<int> &daughterOrder) {
    int q = Utils::randRangeIncl(1, static_cast<int>(motherOrder.size()));

    for(int i=0; i<q; i++)
		daughterOrder[i] = motherOrder[i];

    int ctr = q;
    for(int i=0; i<fatherOrder.size(); i++) {
        if(!Utils::rangeContains(motherOrder, 0, q-1, fatherOrder[i])) {
            daughterOrder[ctr] = fatherOrder[i];
            ctr++;
        }
    }
}

template<class Individual>
void GeneticAlgorithm<Individual>::neighborhoodSwap(vector<int> &order) {
    for(int i=2; i<p.numJobs-1; i++)
        if(Utils::randRangeIncl(1, 100) <= pmutate && !p.adjMx[i-1][i])
            swap(order, i-1, i);
}

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H