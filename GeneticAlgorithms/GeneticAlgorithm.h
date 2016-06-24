//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_GENETICALGORITHMS_H
#define CPP_RCPSP_OC_GENETICALGORITHMS_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <boost/format.hpp>
#include "../ProjectWithOvertime.h"
#include "../Stopwatch.h"

using namespace std;

const bool FORCE_SINGLE_THREAD = true;

enum class SelectionMethod {
	BEST,
	DUEL
};

struct GAParameters {
	GAParameters() {
		numGens = 200;
		popSize = 100;
		pmutate = 5;
		timeLimit = -1.0;
		iterLimit = -1;
		fitnessBasedPairing = false;
		traceobj = false;
		selectionMethod = SelectionMethod::BEST;
        rbbrs = false;
	}

    int numGens, popSize, pmutate;
    double timeLimit;
	int iterLimit;
    bool fitnessBasedPairing, traceobj;
	SelectionMethod selectionMethod;
    bool rbbrs;
};

template<class Individual>
class GeneticAlgorithm {
public:
    virtual ~GeneticAlgorithm();

    pair<vector<int>, float> solve();

    void setParameters(GAParameters _params);

	string getName() const { return name; };

protected:
    GAParameters params;
	ProjectWithOvertime &p;

    Utils::Tracer *tr;   

	// also consider FORCE_SINGLE_THREAD!
    bool useThreads = false;
	const string name;

    GeneticAlgorithm(ProjectWithOvertime &_p, string _name = "GenericGA") : p(_p), tr(nullptr), name(_name) {}

	void generateChildren(vector<pair<Individual, float>> & population);

    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;
	virtual vector<int> decode(Individual &i) = 0;	

    pair<int, int> computePair(vector<pair<Individual, float>> &pop, vector<bool> &alreadySelected);

    void mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx);

	void selectBest(vector<pair<Individual, float>> &pop);
	void selectDuel(vector<pair<Individual, float>> &pop);

    template<class Func>
    void withMutProb(Func code);
};

template<class Individual>
GeneticAlgorithm<Individual>::~GeneticAlgorithm() {
    if(tr != nullptr) delete tr;
}

template<class Individual>
template<class Func>
inline void GeneticAlgorithm<Individual>::withMutProb(Func code) {
    if(Utils::randRangeIncl(1, 100) <= params.pmutate) { code(); }
}

template<class Individual>
void GeneticAlgorithm<Individual>::setParameters(GAParameters _params) {
    params = _params;
    if(params.traceobj && tr == nullptr) {
        tr = new Utils::Tracer("GA"+name+"Trace_" + p.instanceName);
    }
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
    vector<bool> alreadySelected(params.popSize, false);

    for(int childIx=params.popSize; childIx<params.popSize*2; childIx +=2) {
        pair<int, int> parentIndices = computePair(pop, alreadySelected);
        alreadySelected[parentIndices.first] = true;
        alreadySelected[parentIndices.second] = true;
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
void GeneticAlgorithm<Individual>::selectBest(vector<pair<Individual, float>> &pop) {
	sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });
}

template<class T>
void swapIndividuals(vector<T> &pop, int ix1, int ix2) {
	T tmp = pop[ix1];
	pop[ix1] = pop[ix2];
	pop[ix2] = pop[ix1];
}

template<class T>
void swapBestIndividualToFront(vector<T> &pop) {
	int ixOfBest = 0;
	for (int i = 1; i < pop.size() / 2; i++) {
		if (pop[i].second < pop[ixOfBest].second) {
			ixOfBest = i;
		}
	}
	swapIndividuals(pop, 0, ixOfBest);
}

template<class Individual>
void GeneticAlgorithm<Individual>::selectDuel(vector<pair<Individual, float>> &pop) {
	vector<bool> alreadySelected(params.popSize*2, false);

	for (int i = 0; i < params.popSize; i++) {
		pair<int, int> p;
		do {
			p.first = Utils::randRangeIncl(0, params.popSize - 1);
		} while (alreadySelected[p.first]);
		do {
			p.second = Utils::randRangeIncl(params.popSize, params.popSize * 2 - 1);
		} while (alreadySelected[p.second] || p.first == p.second);

		if(pop[p.first].second > pop[p.second].second) {
			swapIndividuals(pop, p.first, p.second);
		}

        alreadySelected[p.first] = true;
        alreadySelected[p.second] = true;
	}

	swapBestIndividualToFront(pop);
}

template<class Individual>
pair<vector<int>, float> GeneticAlgorithm<Individual>::solve() {
    assert(params.pmutate > 0);
    assert(params.popSize > 0);

	Stopwatch sw;
	sw.start();
    vector<pair<Individual, float>> pop(params.popSize*2);

    const int NUM_THREADS = 4;
    thread *threads[NUM_THREADS];
    int numPerThread = params.popSize / NUM_THREADS;

	// Compute initial population
    for(int i=0; i<params.popSize*2; i++) {
        pop[i].first = init(i);
		pop[i].second = i < params.popSize ? -fitness(pop[i].first) : 0.0f;
    }

	TimePoint lupdate = chrono::system_clock::now();

    float lastBestVal = numeric_limits<float>::max();

    for(int i=0;   (params.iterLimit == -1 || i * params.popSize < params.iterLimit)
				&& (params.numGens == -1 || i<params.numGens)
				&& (params.timeLimit == -1.0 || sw.look() < params.timeLimit * 1000.0); i++) {
		if (chrono::duration<double, std::milli>(chrono::system_clock::now() - lupdate).count() > MSECS_BETWEEN_TRACES) {
			cout << "Generations = " << (i + 1) << ", Obj = " << -pop[0].second << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
			lupdate = chrono::system_clock::now();
            if(params.traceobj) tr->trace(sw.look(), -pop[0].second);
		}

		// Pairing and crossover
        generateChildren(pop);

		// Mutation and fitness computation
        if(useThreads && !FORCE_SINGLE_THREAD) {
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

		// Selection
		switch(params.selectionMethod) {
		case SelectionMethod::BEST:
			selectBest(pop);
			break;
		case SelectionMethod::DUEL:
			selectDuel(pop);
			break;
		}

		//cout << "\rGeneration " << (i + 1) << " Obj=" << -pop[0].second << " Time=" << (boost::format("%.2f") % (sw.look() / 1000.0)) << "       ";

		// Show improvements
        if(pop[0].second < lastBestVal) {
            if(lastBestVal == numeric_limits<float>::max())
                cout << "Initial improvement by " << to_string(-pop[0].second) << endl;
			else
	            cout << "Improvement by " << to_string(lastBestVal - pop[0].second) << endl;

	        if(params.traceobj)
				tr->trace(sw.look(), -pop[0].second);
        }
        lastBestVal = pop[0].second;
    }

    auto best = pop[0];
	return make_pair(decode(best.first), -best.second);
}



#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
