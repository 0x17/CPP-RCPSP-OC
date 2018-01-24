//
// Created by André Schnabel on 25.10.15.
//

#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <thread>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include "../ProjectWithOvertime.h"
#include "../Stopwatch.h"
#include "../BasicSolverParameters.h"
#include "../Logger.h"

const bool FORCE_SINGLE_THREAD = true;

enum class SelectionMethod {
	BEST,
	DUEL
};

enum class CrossoverMethod {
	OPC,
	TPC
};

enum class ScheduleGenerationScheme {
	SERIAL,
	PARALLEL
};

inline std::string traceFilenameForGeneticAlgorithm(const std::string &outPath, const std::string &gaName, const std::string &instanceName) {
	return outPath + "GA" + gaName + "Trace_" + instanceName;
}

struct GAParameters : BasicSolverParameters {
	GAParameters();

	void fromJsonFile(const std::string &fn = "GAParameters.json");
	void fromJsonStr(const std::string &s);

	int numGens, popSize, pmutate;
    bool fitnessBasedPairing;
	SelectionMethod selectionMethod;
	CrossoverMethod crossoverMethod;
	ScheduleGenerationScheme sgs;
	bool rbbrs, enforceTopOrdering, fbiFeedbackInjection;
};

struct FitnessResult {
	float value;
	int numSchedulesGenerated;

	FitnessResult(float value, int numSchedulesGenerated);
	FitnessResult();
};

template<class Individual>
class GeneticAlgorithm {
public:
    virtual ~GeneticAlgorithm() = default;

    std::pair<std::vector<int>, float> solve();

    void setParameters(GAParameters _params);

	std::string getName() const { return name; }

protected:
    GAParameters params;
	ProjectWithOvertime &p;

    std::unique_ptr<Utils::Tracer> tr = nullptr;

	// also consider FORCE_SINGLE_THREAD!
    bool useThreads = false;
	const std::string name;

    explicit GeneticAlgorithm(ProjectWithOvertime &_p, const std::string &_name = "GenericGA") : p(_p), tr(nullptr), name(_name) {}

    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;

    virtual FitnessResult fitness(Individual &i) = 0;
	virtual std::vector<int> decode(Individual &i) = 0;
	
    template<class Func>
    void withMutProb(Func code) const;

private:
	std::vector<std::pair<Individual, float>> computeInitialPopulation(int popSize, int &scheduleCount, int &indivCount);
	void generateChildren(std::vector<std::pair<Individual, float>> & population);

	std::pair<int, int> computePair(const std::vector<bool> &alreadySelected) const;
	std::pair<int, int> mutateAndFitnessRange(std::vector<std::pair<Individual, float>> *pop, int startIx, int endIx);

	void selectBest(std::vector<std::pair<Individual, float>> &pop);
	void selectDuel(std::vector<std::pair<Individual, float>> &pop);	
};

template<class Individual>
template<class Func>
inline void GeneticAlgorithm<Individual>::withMutProb(Func code) const {
    if(Utils::randRangeIncl(1, 100) <= params.pmutate) { code(); }
}

template<class Individual>
void GeneticAlgorithm<Individual>::setParameters(GAParameters _params) {
    params = _params;
    if(params.traceobj && tr == nullptr) {
        tr = std::make_unique<Utils::Tracer>(traceFilenameForGeneticAlgorithm(params.outPath, name, p.instanceName));
    }
}

template<class Individual>
std::pair<int, int> GeneticAlgorithm<Individual>::computePair(const std::vector<bool> &alreadySelected) const {
    std::pair<int, int> p;

    do {
        p.first = Utils::randRangeIncl(0, params.fitnessBasedPairing ? (params.popSize / 2) - 1 : params.popSize-1);
    } while(alreadySelected[p.first]);
    do {
        p.second = params.fitnessBasedPairing ? Utils::randRangeIncl(params.popSize / 2, params.popSize-1) : Utils::randRangeIncl(0, params.popSize-1);
    } while(alreadySelected[p.second] || p.first == p.second);

    return p;
};

template<class Individual>
void GeneticAlgorithm<Individual>::generateChildren(std::vector<std::pair<Individual, float>> & pop) {
	std::vector<bool> alreadySelected(params.popSize, false);

    for(int childIx=params.popSize; childIx<params.popSize*2; childIx +=2) {
		std::pair<int, int> parentIndices = computePair(alreadySelected);
        alreadySelected[parentIndices.first] = true;
        alreadySelected[parentIndices.second] = true;
        crossover(pop[parentIndices.first].first, pop[parentIndices.second].first, pop[childIx].first);
        crossover(pop[parentIndices.second].first, pop[parentIndices.first].first, pop[childIx+1].first);
    }
}

template<class Individual>
std::pair<int,int> GeneticAlgorithm<Individual>::mutateAndFitnessRange(std::vector<std::pair<Individual, float>> *pop, int startIx, int endIx) {
	static FitnessResult fres;
	int scheduleCount = 0, indivCount = 0;
    for(int i=startIx; i<=endIx; i++) {
        mutate((*pop)[i].first);
		fres = fitness((*pop)[i].first);
        (*pop)[i].second = -fres.value;
		scheduleCount += fres.numSchedulesGenerated;
		indivCount++;
    }
	return { scheduleCount, indivCount };
}

template<class Individual>
void GeneticAlgorithm<Individual>::selectBest(std::vector<std::pair<Individual, float>> &pop) {
	std::sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });
}

template<class T>
void swapIndividuals(std::vector<T> &pop, int ix1, int ix2) {
	T tmp = pop[ix1];
	pop[ix1] = pop[ix2];
	pop[ix2] = pop[ix1];
}

template<class T>
void swapBestIndividualToFront(std::vector<T> &pop) {
	int ixOfBest = 0;
	for (int i = 1; i < pop.size() / 2; i++) {
		if (pop[i].second < pop[ixOfBest].second) {
			ixOfBest = i;
		}
	}
	swapIndividuals(pop, 0, ixOfBest);
}

template<class Individual>
void GeneticAlgorithm<Individual>::selectDuel(std::vector<std::pair<Individual, float>> &pop) {
	std::vector<bool> alreadySelected(params.popSize*2, false);

	for (int i = 0; i < params.popSize; i++) {
		std::pair<int, int> p;
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
std::vector<std::pair<Individual, float>> GeneticAlgorithm<Individual>::computeInitialPopulation(int popSize, int &scheduleCount, int &indivCount) {
	std::vector<std::pair<Individual, float>> pop(popSize*2);

	LOG_I("Computing initial population");

	for (int i = 0; i<params.popSize * 2; i++) {
		pop[i].first = init(i);

		if (i < params.popSize) {
			FitnessResult fres = fitness(pop[i].first);
			pop[i].second = -fres.value;
			scheduleCount += fres.numSchedulesGenerated;
			indivCount++;

			if (pop[i].second < pop[0].second) {
				std::pair<Individual, float> tmpIndiv = pop[0];
				pop[0] = pop[i];
				pop[i] = tmpIndiv;
			}

			if (tr != nullptr && indivCount > 0) {
				tr->countTrace(-pop[0].second, scheduleCount, indivCount);
				tr->intervalTrace(-pop[0].second, scheduleCount, indivCount);
			}
		}
		else {
			pop[i].second = 0.0f;
		}
	}

	return pop;
}

template<class Individual>
std::pair<std::vector<int>, float> GeneticAlgorithm<Individual>::solve() {
	if(tr != nullptr)
		tr->setTraceMode(Utils::Tracer::TraceMode::ONLY_COUNT);

    assert(params.pmutate > 0);
    assert(params.popSize > 0);

	Stopwatch sw;
	sw.start();

    const int NUM_THREADS = 4;
    std::thread *threads[NUM_THREADS];
    int numPerThread = params.popSize / NUM_THREADS;

	int scheduleCount = 0, indivCount = 0;
	std::vector<std::pair<Individual, float>> pop = computeInitialPopulation(params.popSize, scheduleCount, indivCount);
    float lastBestVal = std::numeric_limits<float>::max();

	/*auto iterationLogger = [&](int i) {
		static TimePoint lupdate = chrono::system_clock::now();		
		double deltat = chrono::duration<double, milli>(chrono::system_clock::now() - lupdate).count();
		if ((sw.look() <= 1000.0 && deltat >= MSECS_BETWEEN_TRACES_SHORT) || (sw.look() > 1000.0 && deltat >= MSECS_BETWEEN_TRACES_LONG)) {
			cout << "Generations = " << (i + 1) << ", Obj = " << -pop[0].second << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
			lupdate = chrono::system_clock::now();
			if(params.traceobj) tr->trace(sw.look(), -pop[0].second);
		}
	};*/

	LOG_I("Computing with abort criterias: iterLimit=" + std::to_string(params.iterLimit) + ", numGens=" + std::to_string(params.numGens) + ", timeLimit=" + std::to_string(params.timeLimit));
    for(int i=0;   (params.iterLimit == -1 || scheduleCount <= params.iterLimit)
				&& (params.numGens == -1 || i < params.numGens)
				&& (params.timeLimit == -1.0 || sw.look() < params.timeLimit * 1000.0); i++) {

		if (tr != nullptr) {
			tr->intervalTrace(-pop[0].second, scheduleCount, indivCount);
		}

		//iterationLogger(i);

		// Pairing and crossover
        generateChildren(pop);

		// Mutation and fitness computation
        if(useThreads && !FORCE_SINGLE_THREAD) {
            for(int tix = 0; tix < NUM_THREADS; tix++) {
                int six = params.popSize+tix*numPerThread;
                int eix = params.popSize+(tix+1)*numPerThread-1;
                threads[tix] = new std::thread(&GeneticAlgorithm<Individual>::mutateAndFitnessRange, this, &pop, six, eix);
            }

            for(auto thrd : threads) {
                thrd->join();
                delete thrd;
            }
        } else {
            for(int j=params.popSize; j<params.popSize*2; j++) {
                mutate(pop[j].first);
				FitnessResult fres = fitness(pop[j].first);
                pop[j].second = -fres.value;
				scheduleCount += fres.numSchedulesGenerated;
				indivCount++;
				if(tr != nullptr)
					tr->countTrace(-pop[0].second, scheduleCount, indivCount);
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
			if (lastBestVal == std::numeric_limits<float>::max())
				LOG_I("Initial improvement by " + std::to_string(-pop[0].second));
			else
				LOG_I("Improvement by " + std::to_string(lastBestVal - pop[0].second));

	        /*if(params.traceobj)
				tr->trace(sw.look(), -pop[0].second, scheduleCount, indivCount);*/
        }
        lastBestVal = pop[0].second;
    }

	if (tr != nullptr) {
		tr->intervalTrace(-pop[0].second, scheduleCount, indivCount);
		tr->countTrace(-pop[0].second, scheduleCount, indivCount);
	}

    auto best = pop[0];
	return std::make_pair(decode(best.first), -best.second);
}
