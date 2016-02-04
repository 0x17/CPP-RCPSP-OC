//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"
#include "Sampling.h"
#include "GeneticOperators.h"

TimeWindowArbitraryGA::TimeWindowArbitraryGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeWindowArbitraryGA") {
    useThreads = false;
}

LambdaTau TimeWindowArbitraryGA::init(int ix) {
    LambdaTau indiv(p);
    indiv.order = ix == 0 ? p.topOrder : Sampling::naiveSampling(p);
    p.eachJob([&](int j) { indiv.tau[j] = ix == 0 ? 0.0f : Utils::randUnitFloat(); });
    return indiv;
}

void TimeWindowArbitraryGA::crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) {
	GeneticOperators::randomOnePointCrossoverAssociated<vector<float>>({ mother.order, father.order, daughter.order }, { mother.tau, father.tau, daughter.tau });
}

void TimeWindowArbitraryGA::mutate(LambdaTau &i) {
	GeneticOperators::neighborhoodSwapAssociated(p.adjMx, i.order, i.tau, params.pmutate);
	p.eachJob([&](int j) {
        withMutProb([&] {
            i.tau[j] = 1.0f - i.tau[j];
        });
    });
}

float TimeWindowArbitraryGA::fitness(LambdaTau &i) {
    auto pair = p.serialSGSTimeWindowArbitrary(i.order, i.tau);
    return profitForSGSResult(pair);
}

vector<int> TimeWindowArbitraryGA::decode(LambdaTau& i)  {
	return p.serialSGSTimeWindowArbitrary(i.order, i.tau).first;
}

//======================================================================================================================

TimeWindowBordersGA::TimeWindowBordersGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeWindowBordersGA") {
    useThreads = false;
}

LambdaBeta TimeWindowBordersGA::init(int ix) {
    LambdaBeta indiv(p);
	indiv.order = ix == 0 ? p.topOrder : Sampling::naiveSampling(p);
    p.eachJob([&](int j) { indiv.beta[j] = ix == 0 ? 0 : rand() % 2; });
    return indiv;
}

void TimeWindowBordersGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
	GeneticOperators::randomOnePointCrossoverAssociated<vector<int>>({ mother.order, father.order, daughter.order }, { mother.beta, father.beta, daughter.beta });
}

void TimeWindowBordersGA::mutate(LambdaBeta &i) {
	GeneticOperators::neighborhoodSwapAssociated(p.adjMx, i.order, i.beta, params.pmutate);
	p.eachJob([&](int j) {
        withMutProb([&] {
            i.beta[j] = 1-i.beta[j];
        });
    });
}

float TimeWindowBordersGA::fitness(LambdaBeta &i) {
    auto pair = p.serialSGSTimeWindowBorders(i.order, i.beta);
    return profitForSGSResult(pair);
}

vector<int> TimeWindowBordersGA::decode(LambdaBeta& i) {
	return p.serialSGSTimeWindowBorders(i.order, i.beta).first;
}

//======================================================================================================================

CompareAlternativesGA::CompareAlternativesGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "CompareAlternativesGA") {
    useThreads = true;
}

vector<int> CompareAlternativesGA::init(int ix) {
    return ix == 0 ? p.topOrder : Sampling::naiveSampling(p);
}

void CompareAlternativesGA::crossover(vector<int> &mother, vector<int> &father, vector<int> &daughter) {
	GeneticOperators::randomOnePointCrossover({ mother, father, daughter });
}

void CompareAlternativesGA::mutate(vector<int> &i) {
	GeneticOperators::neighborhoodSwap(p.adjMx, i, params.pmutate);
}

float CompareAlternativesGA::fitness(vector<int> &i) {
    auto pair = p.serialSGSWithOvertime(i);
    return profitForSGSResult(pair);
}

vector<int> CompareAlternativesGA::decode(vector<int>& i) {
	return p.serialSGSWithOvertime(i).first;
}
