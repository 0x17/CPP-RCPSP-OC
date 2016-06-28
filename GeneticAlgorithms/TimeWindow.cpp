//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"
#include "Sampling.h"

TimeWindowArbitraryDiscretizedGA::TimeWindowArbitraryDiscretizedGA(ProjectWithOvertime &_p, int _ub) : GeneticAlgorithm(_p, "TimeWindowArbitraryGA"), ub(_ub) {
	useThreads = false;
}

LambdaBeta TimeWindowArbitraryDiscretizedGA::init(int ix) {
	LambdaBeta indiv(p.numJobs);
	indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
	p.eachJob([&](int j) { indiv.beta[j] = ix == 0 ? 0 : Utils::randRangeIncl(0, ub); });
	return indiv;
}

void TimeWindowArbitraryDiscretizedGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
	daughter.randomOnePointCrossover(mother, father);
}

void TimeWindowArbitraryDiscretizedGA::mutate(LambdaBeta &i) {
	i.neighborhoodSwap(p.adjMx, params.pmutate);
	p.eachJob([&](int j) {
		withMutProb([&] {
			i.beta[j] = Utils::randRangeIncl(0, ub);
		});
	});
}

float TimeWindowArbitraryDiscretizedGA::fitness(LambdaBeta &i) {
	vector<float> tau = Utils::constructVector<float>(i.beta.size(), [&i, this](int ix) {
		return static_cast<float>(static_cast<double>(i.beta[ix]) / static_cast<double>(ub - 1));
	});
	auto pair = p.serialSGSTimeWindowArbitrary(i.order, tau);
	return p.calcProfit(pair);
}

vector<int> TimeWindowArbitraryDiscretizedGA::decode(LambdaBeta& i) {
	vector<float> tau = Utils::constructVector<float>(i.beta.size(), [&i, this](int ix) {
		return static_cast<float>(static_cast<double>(i.beta[ix]) / static_cast<double>(ub - 1));
	});
	return p.serialSGSTimeWindowArbitrary(i.order, tau).sts;
}

//======================================================================================================================

TimeWindowArbitraryGA::TimeWindowArbitraryGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeWindowArbitraryGA") {
    useThreads = false;
}

LambdaTau TimeWindowArbitraryGA::init(int ix) {
    LambdaTau indiv(p.numJobs);
    indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    p.eachJob([&](int j) { indiv.tau[j] = ix == 0 ? 0.0f : Utils::randUnitFloat(); });
    return indiv;
}

void TimeWindowArbitraryGA::crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) {
    daughter.randomOnePointCrossover(mother, father);
}

void TimeWindowArbitraryGA::mutate(LambdaTau &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
	p.eachJob([&](int j) {
        withMutProb([&] {
            //i.tau[j] = 1.0f - i.tau[j];
			i.tau[j] = Utils::randUnitFloat();
        });
    });
}

float TimeWindowArbitraryGA::fitness(LambdaTau &i) {
    auto pair = p.serialSGSTimeWindowArbitrary(i.order, i.tau);
	return p.calcProfit(pair);
}

vector<int> TimeWindowArbitraryGA::decode(LambdaTau& i)  {
	return p.serialSGSTimeWindowArbitrary(i.order, i.tau).sts;
}

//======================================================================================================================

ProjectWithOvertime::BorderSchedulingOptions TimeWindowBordersGA::options;

void TimeWindowBordersGA::setVariant(int variant) {
	options.setFromIndex(variant);
	LambdaBeta::setOptions(options);
}

TimeWindowBordersGA::TimeWindowBordersGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeWindowBordersGA") {
    useThreads = false;
}

LambdaBeta TimeWindowBordersGA::init(int ix) {
    LambdaBeta indiv(p.numJobs);
    indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    p.eachJob([&](int j) { indiv.beta[j] = ix == 0 ? 0 : Utils::randRangeIncl(0, 1); });
    return indiv;
}

void TimeWindowBordersGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
	if (options.separateCrossover) daughter.separateOnePointCrossover(mother, father);
	else daughter.randomOnePointCrossover(mother, father);
}

void TimeWindowBordersGA::mutate(LambdaBeta &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
	p.eachJob([&](int j) {
        withMutProb([&] {
            i.beta[j] = 1-i.beta[j];
        });
    });
}

float TimeWindowBordersGA::fitness(LambdaBeta &i) {
    auto pair = p.serialSGSTimeWindowBorders(i.order, i.beta, options);
	return p.calcProfit(pair);
}

vector<int> TimeWindowBordersGA::decode(LambdaBeta& i) {
	return p.serialSGSTimeWindowBorders(i.order, i.beta, options).sts;
}

//======================================================================================================================

CompareAlternativesGA::CompareAlternativesGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "CompareAlternativesGA") {
    useThreads = true;
}

Lambda CompareAlternativesGA::init(int ix) {
    Lambda l;
    l.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    return l;
}

void CompareAlternativesGA::crossover(Lambda &mother, Lambda &father, Lambda &daughter) {
    daughter.randomOnePointCrossover(mother, father);
}

void CompareAlternativesGA::mutate(Lambda &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
}

float CompareAlternativesGA::fitness(Lambda &i) {
    auto pair = p.serialSGSWithOvertime(i.order);
	return p.calcProfit(pair);
}

vector<int> CompareAlternativesGA::decode(Lambda& i) {
	return p.serialSGSWithOvertime(i.order).sts;
}

