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

using std::string;
using std::vector;
using std::pair;
using std::to_string;

const bool FORCE_SINGLE_THREAD = true;

enum class SelectionMethod {
	BEST,
	DUEL
};

inline string traceFilenameForGeneticAlgorithm(const string &outPath, const string &gaName, const string &instanceName) {
	return outPath + "GA" + gaName + "Trace_" + instanceName;
}

struct GAParameters : Utils::BasicSolverParameters {
	GAParameters();
	void parseFromString(string s);
	void parseFromDisk(string fn = "GAParameters.txt");

	int numGens, popSize, pmutate;
    bool fitnessBasedPairing;
	SelectionMethod selectionMethod;
    bool rbbrs;
};

inline GAParameters::GAParameters() :
	BasicSolverParameters(-1.0, -1, false, "GATrace_", 1),
	numGens(200),
	popSize(100),
	pmutate(5),
	fitnessBasedPairing(false),
	selectionMethod(SelectionMethod::BEST),
	rbbrs(false) {
}

inline void GAParameters::parseFromString(string s) {
	vector<string> lines, parts;
	boost::split(lines, s, boost::is_any_of("\n"));
	for(auto line : lines) {
		if (!boost::contains(line, "=")) continue;

		boost::split(parts, line, boost::is_any_of("="));		
		if (boost::equals(parts[0], "numGens"))
			numGens = stoi(parts[1]);
		else if (boost::equals(parts[0], "popSize"))
			popSize = stoi(parts[1]);
		else if (boost::equals(parts[0], "pmutate"))
			pmutate = stoi(parts[1]);
		else if (boost::equals(parts[0], "timeLimit"))
			timeLimit = stod(parts[1]);
		else if (boost::equals(parts[0], "iterLimit"))
			iterLimit = stoi(parts[1]);
		else if (boost::equals(parts[0], "fitnessBasedPairing"))
			fitnessBasedPairing = boost::equals(parts[1], "true");
		else if (boost::equals(parts[0], "selectionMethod"))
			selectionMethod = (boost::equals(parts[1], "best")) ? SelectionMethod::BEST : SelectionMethod::DUEL;
		else if (boost::equals(parts[0], "rbbrs"))
			rbbrs = boost::equals(parts[1], "true");
	}
}

inline void GAParameters::parseFromDisk(string fn) {
	if(boost::filesystem::exists(fn))
		parseFromString(Utils::slurp(fn));
}

struct FitnessResult {
	float value;
	int numSchedulesGenerated;

	FitnessResult(float value, int numSchedulesGenerated)
			: value(value),
			  numSchedulesGenerated(numSchedulesGenerated) {}

	FitnessResult() : value(0.0f), numSchedulesGenerated(0) {}
};

template<class Individual>
class GeneticAlgorithm {
public:
    virtual ~GeneticAlgorithm();

    pair<vector<int>, float> solve();

    void setParameters(GAParameters _params);

	string getName() const { return name; }

protected:
    GAParameters params;
	ProjectWithOvertime &p;

    std::unique_ptr<Utils::Tracer> tr = nullptr;   

	// also consider FORCE_SINGLE_THREAD!
    bool useThreads = false;
	const string name;

    GeneticAlgorithm(ProjectWithOvertime &_p, string _name = "GenericGA") : p(_p), tr(nullptr), name(_name) {}

	void generateChildren(vector<pair<Individual, float>> & population);

    virtual Individual init(int ix) = 0;
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;

    virtual FitnessResult fitness(Individual &i) = 0;

	virtual vector<int> decode(Individual &i) = 0;	

    pair<int, int> computePair(const vector<bool> &alreadySelected);

    int mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx);

	void selectBest(vector<pair<Individual, float>> &pop);
	void selectDuel(vector<pair<Individual, float>> &pop);

    template<class Func>
    void withMutProb(Func code) const;
};

template<class Individual>
GeneticAlgorithm<Individual>::~GeneticAlgorithm() {
}

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
pair<int, int> GeneticAlgorithm<Individual>::computePair(const vector<bool> &alreadySelected) {
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
        pair<int, int> parentIndices = computePair(alreadySelected);
        alreadySelected[parentIndices.first] = true;
        alreadySelected[parentIndices.second] = true;
        crossover(pop[parentIndices.first].first, pop[parentIndices.second].first, pop[childIx].first);
        crossover(pop[parentIndices.second].first, pop[parentIndices.first].first, pop[childIx+1].first);
    }
}

template<class Individual>
int GeneticAlgorithm<Individual>::mutateAndFitnessRange(vector<pair<Individual, float>> *pop, int startIx, int endIx) {
	static FitnessResult fres;
	int scheduleCount = 0;
    for(int i=startIx; i<=endIx; i++) {
        mutate((*pop)[i].first);
		fres = fitness((*pop)[i].first);
        (*pop)[i].second = -fres.value;
		scheduleCount += fres.numSchedulesGenerated;
    }
	return scheduleCount;
}

template<class Individual>
void GeneticAlgorithm<Individual>::selectBest(vector<pair<Individual, float>> &pop) {
	std::sort(pop.begin(), pop.end(), [](auto &left, auto &right) { return left.second < right.second; });
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
    std::thread *threads[NUM_THREADS];
    int numPerThread = params.popSize / NUM_THREADS;

	int scheduleCount = 0;

	LOG_I("Computing initial population");
	FitnessResult fres;
	pair<Individual, float> tmpIndiv;
    for(int i=0; i<params.popSize*2; i++) {
        pop[i].first = init(i);
		fres = fitness(pop[i].first);
		pop[i].second = i < params.popSize ? -fres.value : 0.0f;
		scheduleCount += fres.numSchedulesGenerated;

		if(pop[i].second < pop[0].second) {
			tmpIndiv = pop[0];
			pop[0] = pop[i];
			pop[i] = tmpIndiv;
			if(tr != nullptr) {
				tr->intervalTrace(-pop[0].second);
			}
		}
    }	

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

	LOG_I("Computing with abort criterias: iterLimit=" + to_string(params.iterLimit) + ", numGens=" + to_string(params.numGens) + ", timeLimit=" + to_string(params.timeLimit));
    for(int i=0;   (params.iterLimit == -1 || scheduleCount <= params.iterLimit)
				&& (params.numGens == -1 || i<params.numGens)
				&& (params.timeLimit == -1.0 || sw.look() < params.timeLimit * 1000.0); i++) {

		if (tr != nullptr) {
			tr->intervalTrace(-pop[0].second);
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
				fres = fitness(pop[j].first);
                pop[j].second = -fres.value;
				scheduleCount += fres.numSchedulesGenerated;
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
				LOG_I("Initial improvement by " + to_string(-pop[0].second));
			else
				LOG_I("Improvement by " + to_string(lastBestVal - pop[0].second));

	        //if(params.traceobj)
			//	tr->trace(sw.look(), -pop[0].second);
        }
        lastBestVal = pop[0].second;
    }

	if (tr != nullptr) {
		tr->intervalTrace(-pop[0].second);
	}

    auto best = pop[0];
	return std::make_pair(decode(best.first), -best.second);
}
