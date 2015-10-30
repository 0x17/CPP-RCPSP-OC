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
    ProjectWithOvertime &p;
    const int numGens, popSize, pmutate;
public:
	virtual ~GeneticAlgorithm() {}

	Individual solve() {
        vector<Individual> population(popSize*2);

        for(int i=0; i<popSize; i++) {
            population[i] = init(i);
        }

        for(int i=0; i<numGens; i++) {
            // Crossover 2 parents (0..popSize-1) into children (popSize..popSize*2-1)
            // Mutate children
            // Compute fitness for all
            // Sort descending fitness
        }

        return population[0];
    }

protected:
    GeneticAlgorithm(ProjectWithOvertime &_p) : p(_p), numGens(80), pmutate(5), popSize(100) {}
    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;

    void neighborhoodSwap(vector<int> &order) {
        for(int i=2; i<p.numJobs-1; i++)
            if(Utils::randRangeIncl(1, 100) <= pmutate && !p.adjMx[i-1][i])
                swap(order, i-1, i);
    }

    void onePointCrossover(vector<int> &motherOrder, vector<int> &fatherOrder, vector<int> &daughterOrder) {
        int q = Utils::randRangeIncl(1, motherOrder.size());

        for(int i=0; i<q; i++) daughterOrder[i] = motherOrder[i];

        int ctr = q;
        for(int i=0; i<fatherOrder.size(); i++) {
            if(!Utils::rangeContains(motherOrder, 0, q-1, fatherOrder[i])) {
                daughterOrder[ctr] = fatherOrder[i];
                ctr++;
            }
        }
    }

    inline void swap(vector<int> &order, int i1, int i2) {
        int tmp = order[i1];
        order[i1] = order[i2];
        order[i2] = tmp;
    }
};

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
