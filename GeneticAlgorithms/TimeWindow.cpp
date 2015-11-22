//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"
#include "Sampling.h"

TimeWindowArbitraryGA::TimeWindowArbitraryGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) {
    useThreads = false;
}

LambdaTau TimeWindowArbitraryGA::init(int ix) {
    LambdaTau indiv(p);
    indiv.order = Sampling::naiveSampling(p);
    p.eachJob([&](int j) { indiv.tau[j] = Utils::randUnitFloat(); });
    return indiv;
}

void TimeWindowArbitraryGA::crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) {
	onePointCrossoverAssociated<vector<int>, vector<float>>({ mother.order, father.order, daughter.order }, { mother.tau, father.tau, daughter.tau });
}

void TimeWindowArbitraryGA::mutate(LambdaTau &i) {
	neighborhoodSwapAssociated(i.order, i.tau);
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

TimeWindowBordersGA::TimeWindowBordersGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) {
    useThreads = false;
}

LambdaBeta TimeWindowBordersGA::init(int ix) {
    LambdaBeta indiv(p);
    indiv.order = Sampling::naiveSampling(p);
    p.eachJob([&](int j) { indiv.beta[j] = rand() % 2; });
    return indiv;
}

void TimeWindowBordersGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
	onePointCrossoverAssociated<vector<int>, vector<int>>({ mother.order, father.order, daughter.order }, { mother.beta, father.beta, daughter.beta });
}

void TimeWindowBordersGA::mutate(LambdaBeta &i) {
	neighborhoodSwapAssociated(i.order, i.beta);
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

CompareAlternativesGA::CompareAlternativesGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) {
    useThreads = true;
}

vector<int> CompareAlternativesGA::init(int ix) {
    return Sampling::naiveSampling(p);
}

void CompareAlternativesGA::crossover(vector<int> &mother, vector<int> &father, vector<int> &daughter) {
	onePointCrossover({ mother, father, daughter });
}

void CompareAlternativesGA::mutate(vector<int> &i) {
    neighborhoodSwap(i);
}

float CompareAlternativesGA::fitness(vector<int> &i) {
    auto pair = p.serialSGSWithOvertime(i);
    return profitForSGSResult(pair);
}

vector<int> CompareAlternativesGA::decode(vector<int>& i) {
	return p.serialSGSWithOvertime(i).first;
}

string TimeWindowBordersGA::getName() const {
    return "TimeWindowBordersGA";
}

string TimeWindowArbitraryGA::getName() const {
    return "TimeWindowArbitraryGA";
}

string CompareAlternativesGA::getName() const {
    return "CompareAlternativesGA";
}
