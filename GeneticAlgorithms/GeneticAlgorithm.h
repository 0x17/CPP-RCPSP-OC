//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_GENETICALGORITHMS_H
#define CPP_RCPSP_OC_GENETICALGORITHMS_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include "../ProjectWithOvertime.h"
#include "../Stopwatch.h"

using namespace std;

template<class Individual>
class GeneticAlgorithm {
public:
	virtual ~GeneticAlgorithm() {}

    pair<vector<int>, float> solve();

protected:
    ProjectWithOvertime &p;
    const int numGens, popSize, pmutate;
	const double timelimit;

    GeneticAlgorithm(ProjectWithOvertime &_p) : p(_p), numGens(200), pmutate(5), popSize(100), timelimit(-1.0) {}

	void generateChildren(vector<pair<Individual, float>> & population);

    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;
	virtual vector<int> decode(Individual &i) = 0;
	    
	void neighborhoodSwap(vector<int> &order);
	template<class T>
	void neighborhoodSwapAssociated(vector<int> &order, vector<T> &associated);

	template<class T>
	struct CrossoverData {
		T &mother, &father, &daughter;
	};	
    void onePointCrossover(CrossoverData<vector<int>> order);	
	template<class T>
	void onePointCrossoverAssociated(CrossoverData<T> order, CrossoverData<T> associated);

	template<class T>
    inline void swap(vector<T> &order, int i1, int i2);

    pair<int, int> computePair(vector<bool> &alreadySelected);

    float profitForSGSResult(pair<vector<int>, Matrix<int>> &result);

    void mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx);
};

template<class Individual>
float GeneticAlgorithm<Individual>::profitForSGSResult(pair<vector<int>, Matrix<int>> &result) {
    return p.calcProfit(result.first[p.numJobs-1], result.second);
}

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
void GeneticAlgorithm<Individual>::mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx) {
    for(int i=startIx; i<=endIx; i++) {
        mutate((*pop)[i].first);
        (*pop)[i].second = -fitness((*pop)[i].first);
    }
}

template<class Individual>
pair<vector<int>, float> GeneticAlgorithm<Individual>::solve() {
	Stopwatch sw;
	sw.start();
    vector<pair<Individual, float>> pop(popSize*2);

    const int NUM_THREADS = 4;
    thread *threads[NUM_THREADS];
    int numPerThread = popSize / NUM_THREADS;
    const bool USE_THREADS = true;

    for(int i=0; i<popSize*2; i++) {
        pop[i].first = init(i);
		pop[i].second = i < popSize ? -fitness(pop[i].first) : 0.0f;
    }

    for(int i=0; (numGens == -1 || i<numGens) && (timelimit == -1.0 || sw.look() < timelimit); i++) {		
        generateChildren(pop);

        if(USE_THREADS) {
            for(int tix = 0; tix < NUM_THREADS; tix++) {
                int six = popSize+tix*numPerThread;
                int eix = popSize+(tix+1)*numPerThread-1;
                threads[tix] = new thread(&GeneticAlgorithm<Individual>::mutateAndFitnessRange, this, &pop, six, eix);
            }

            for(auto thrd : threads)
                thrd->join();
        } else {
            for(int j=popSize; j<popSize*2; j++) {
                mutate(pop[j].first);
                pop[j].second = -fitness(pop[j].first);
            }
        }

		sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });

		cout << "\rGeneration " << (i + 1) << " Obj=" << -pop[0].second;
    }

    if(USE_THREADS)
        for(auto thrd : threads)
            delete thrd;

    auto best = pop[0];
	return make_pair(decode(best.first), -best.second);
}

template<class Individual>
template<class T>
void GeneticAlgorithm<Individual>::swap(vector<T> &order, int i1, int i2) {
    T tmp = order[i1];
    order[i1] = order[i2];
    order[i2] = tmp;
}

#define OPC_COMMON(assignFromMother, assignFromFather) \
	int q = Utils::randRangeIncl(0, static_cast<int>(order.mother.size()) - 1); \
	for(int i = 0; i <= q; i++) { assignFromMother; } \
	for(int i = 0, ctr = q + 1; i<order.father.size(); i++) { \
		if(!Utils::rangeContains(order.mother, 0, q, order.father[i])) { \
			assignFromFather; \
			ctr++; \
		} \
	}

template<class Individual>
void GeneticAlgorithm<Individual>::onePointCrossover(CrossoverData<vector<int>> order) {
	OPC_COMMON(
		order.daughter[i] = order.mother[i],
		order.daughter[ctr] = order.father[i])
}

template <class Individual>
template <class T>
void GeneticAlgorithm<Individual>::onePointCrossoverAssociated(CrossoverData<T> order, CrossoverData<T> associated) {
	OPC_COMMON(
		order.daughter[i] = order.mother[i];
		associated.daughter[i] = associated.mother[i],
		order.daughter[ctr] = order.father[i];
		associated.daughter[i] = associated.father[i])
}

#define SWAP_COMMON(code) \
	for(int i=1; i<p.numJobs; i++) \
	if(Utils::randRangeIncl(1, 100) <= pmutate && !p.adjMx(order[i - 1], order[i])) { code; }

template<class Individual>
void GeneticAlgorithm<Individual>::neighborhoodSwap(vector<int> &order) {
	SWAP_COMMON(swap(order, i - 1, i))
}

template<class Individual>
template<class T>
void GeneticAlgorithm<Individual>::neighborhoodSwapAssociated(vector<int> &order, vector<T> &associated) {
	SWAP_COMMON(
		swap(order, i - 1, i);
		swap(associated, i - 1, i))
}

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
