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

struct GAParameters {
    int numGens, popSize, pmutate;
    double timeLimit;
    bool fitnessBasedPairing;
};

template<class Individual>
class GeneticAlgorithm {
public:
	virtual ~GeneticAlgorithm() {}

    pair<vector<int>, float> solve();

    void setParameters(GAParameters _params);

    virtual string getName() const = 0;

protected:
    GAParameters params = {200, 100, 5, -1.0, false};

    ProjectWithOvertime &p;

    bool useThreads = false;

    GeneticAlgorithm(ProjectWithOvertime &_p) : p(_p) {}

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
	template<class T, class U>
	void onePointCrossoverAssociated(CrossoverData<T> order, CrossoverData<U> associated);

	template<class T>
    inline void swap(vector<T> &order, int i1, int i2);

    pair<int, int> computePair(vector<pair<Individual, float>> &pop, vector<bool> &alreadySelected);

    float profitForSGSResult(pair<vector<int>, Matrix<int>> &result);

    void mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx);

    template<class Func>
    void withMutProb(Func code);
};

template<class Individual>
template<class Func>
inline void GeneticAlgorithm<Individual>::withMutProb(Func code) {
    if(Utils::randRangeIncl(1, 100) <= params.pmutate) { code(); }
}

template<class Individual>
void GeneticAlgorithm<Individual>::setParameters(GAParameters _params) {
    params = _params;
}

template<class Individual>
float GeneticAlgorithm<Individual>::profitForSGSResult(pair<vector<int>, Matrix<int>> &result) {
    return p.calcProfit(p.makespan(result.first), result.second);
}

template<class Individual>
pair<int, int> GeneticAlgorithm<Individual>::computePair(vector<pair<Individual, float>> &pop, vector<bool> &alreadySelected) {
    pair<int, int> p;

    do {
        p.first = Utils::randRangeIncl(0, params.fitnessBasedPairing ? (params.popSize / 2) - 1 : params.popSize-1);
    } while(alreadySelected[p.first]);
    do {
        p.second = params.fitnessBasedPairing ? Utils::randRangeIncl(params.popSize / 2, params.popSize-1) : Utils::randRangeIncl(0, params.popSize-1);
    } while(alreadySelected[p.second] || p.first == p.second);

    return p;
};

template<class Individual>
void GeneticAlgorithm<Individual>::generateChildren(vector<pair<Individual, float>> & pop) {
    vector<bool> alreadySelected(params.popSize);

    for(int childIx=params.popSize; childIx<params.popSize*2; childIx +=2) {
        pair<int, int> parentIndices = computePair(pop, alreadySelected);
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
    vector<pair<Individual, float>> pop(params.popSize*2);

    const int NUM_THREADS = 4;
    thread *threads[NUM_THREADS];
    int numPerThread = params.popSize / NUM_THREADS;

    for(int i=0; i<params.popSize*2; i++) {
        pop[i].first = init(i);
		pop[i].second = i < params.popSize ? -fitness(pop[i].first) : 0.0f;
    }

    for(int i=0; (params.numGens == -1 || i<params.numGens) && (params.timeLimit == -1.0 || sw.look() < params.timeLimit); i++) {		
        generateChildren(pop);

        if(useThreads) {
            for(int tix = 0; tix < NUM_THREADS; tix++) {
                int six = params.popSize+tix*numPerThread;
                int eix = params.popSize+(tix+1)*numPerThread-1;
                threads[tix] = new thread(&GeneticAlgorithm<Individual>::mutateAndFitnessRange, this, &pop, six, eix);
            }

            for(auto thrd : threads) {
                thrd->join();
                delete thrd;
            }
        } else {
            for(int j=params.popSize; j<params.popSize*2; j++) {
                mutate(pop[j].first);
                pop[j].second = -fitness(pop[j].first);
            }
        }

		sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });

		//cout << "\rGeneration " << (i + 1) << " Obj=" << -pop[0].second << " Time=" << sw.look() << endl;
    }

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
template <class T, class U>
void GeneticAlgorithm<Individual>::onePointCrossoverAssociated(CrossoverData<T> order, CrossoverData<U> associated) {
	OPC_COMMON(
		order.daughter[i] = order.mother[i];
		associated.daughter[i] = associated.mother[i],
		order.daughter[ctr] = order.father[i];
		associated.daughter[i] = associated.father[i])
}

#define SWAP_COMMON(code) \
	for(int i=1; i<p.numJobs; i++) \
	if(Utils::randRangeIncl(1, 100) <= params.pmutate && !p.adjMx(order[i - 1], order[i])) { code; }

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
