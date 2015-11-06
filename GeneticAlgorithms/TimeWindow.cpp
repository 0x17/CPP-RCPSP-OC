//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"
#include "Sampling.h"

LambdaTau TimeWindowArbitraryGA::init(int ix) {
    LambdaTau indiv(p);
    indiv.order = Sampling::naiveSampling(p);
    P_EACH_JOB(indiv.tau[j] = Utils::randUnitFloat())
    return indiv;
}

void TimeWindowArbitraryGA::crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) {
}

void TimeWindowArbitraryGA::mutate(LambdaTau &i) {
	neighborhoodSwapAssociated(i.order, i.tau);
	P_EACH_JOB(if (Utils::randRangeIncl(1, 100) <= pmutate) i.tau[j] = 1.0f - i.tau[j])
}

float TimeWindowArbitraryGA::fitness(LambdaTau &i) {
    auto pair = p.serialSGSTimeWindowArbitrary(i.order, i.tau);
    return profitForSGSResult(pair);
}

vector<int> TimeWindowArbitraryGA::decode(LambdaTau& i)  {
	return p.serialSGSTimeWindowArbitrary(i.order, i.tau).first;
}

//======================================================================================================================

LambdaBeta TimeWindowBordersGA::init(int ix) {
    LambdaBeta indiv(p);
    indiv.order = Sampling::naiveSampling(p);
    P_EACH_JOB(indiv.beta[j] = rand() % 2);
    return indiv;
}

void TimeWindowBordersGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
}

void TimeWindowBordersGA::mutate(LambdaBeta &i) {
	neighborhoodSwapAssociated(i.order, i.beta);
	P_EACH_JOB(if(Utils::randRangeIncl(1, 100) <= pmutate) i.beta[j] = 1-i.beta[j])
}

float TimeWindowBordersGA::fitness(LambdaBeta &i) {
    auto pair = p.serialSGSTimeWindowBorders(i.order, i.beta);
    return profitForSGSResult(pair);
}

vector<int> TimeWindowBordersGA::decode(LambdaBeta& i) {
	vector<int> sts;
	return sts;
}

//======================================================================================================================

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