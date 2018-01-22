//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"
#include "Sampling.h"

using namespace std;

TimeWindowArbitraryDiscretizedGA::TimeWindowArbitraryDiscretizedGA(ProjectWithOvertime &_p, int _ub) : GeneticAlgorithm(_p, "TimeWindowArbitraryDiscretizedGA"), ub(_ub) {
	useThreads = false;
}

LambdaBeta TimeWindowArbitraryDiscretizedGA::init(int ix) {
	LambdaBeta indiv(p.numJobs);
	indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
	p.eachJob([&](int j) { indiv.beta[j] = ix == 0 ? 0 : Utils::randRangeIncl(0, ub-1); });
	return indiv;
}

void TimeWindowArbitraryDiscretizedGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
	daughter.randomOnePointCrossover(mother, father);
}

void TimeWindowArbitraryDiscretizedGA::mutate(LambdaBeta &i) {
	i.neighborhoodSwap(p.adjMx, params.pmutate, params.enforceTopOrdering);
	p.eachJob([&](int j) {
		withMutProb([&] {
			i.beta[j] = Utils::randRangeIncl(0, ub-1);
		});
	});
}

FitnessResult TimeWindowArbitraryDiscretizedGA::fitness(LambdaBeta &i) {
	vector<float> tau = Utils::constructVector<float>(static_cast<int>(i.beta.size()), [&i, this](int ix) {
		return static_cast<float>(static_cast<double>(i.beta[ix]) / static_cast<double>(ub-1));
	});
	auto res = p.serialSGSTimeWindowArbitrary(ActivityListPrioProvider(i.order), tau);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> TimeWindowArbitraryDiscretizedGA::decode(LambdaBeta& i) {
	vector<float> tau = Utils::constructVector<float>(static_cast<int>(i.beta.size()), [&i, this](int ix) {
		return static_cast<float>(static_cast<double>(i.beta[ix]) / static_cast<double>(ub-1));
	});
	return p.serialSGSTimeWindowArbitrary(ActivityListPrioProvider(i.order), tau).sts;
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
    i.neighborhoodSwap(p.adjMx, params.pmutate, params.enforceTopOrdering);
	p.eachJob([&](int j) {
        withMutProb([&] {
			i.tau[j] = Utils::randUnitFloat();
        });
    });
}

FitnessResult TimeWindowArbitraryGA::fitness(LambdaTau &i) {
    auto pair = p.serialSGSTimeWindowArbitrary(ActivityListPrioProvider(i.order), i.tau);
	return { p.calcProfit(pair), pair.numSchedulesGenerated };
}

vector<int> TimeWindowArbitraryGA::decode(LambdaTau& i)  {
	return p.serialSGSTimeWindowArbitrary(ActivityListPrioProvider(i.order), i.tau).sts;
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
    i.neighborhoodSwap(p.adjMx, params.pmutate, params.enforceTopOrdering);
	p.eachJobConst([&](int j) {
		withMutProb([&] {
			i.beta[j] = 1 - i.beta[j];
		});
	});
	
}

FitnessResult TimeWindowBordersGA::fitness(LambdaBeta &i) {
    auto res = p.serialSGSTimeWindowBordersWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.beta, options);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> TimeWindowBordersGA::decode(LambdaBeta& i) {
	return p.serialSGSTimeWindowBordersWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.beta, options).sts;
}

//======================================================================================================================

ActivityListBasedGA::ActivityListBasedGA(ProjectWithOvertime &_p, const std::string &name, TDecoder _decoder) : GeneticAlgorithm(_p, name), decoder(_decoder) {
    useThreads = true;
}

ActivityListBasedGA::TDecoder ActivityListBasedGA::selectDecoder(DecoderType type) {
	switch(type) {
	case DecoderType::CompareAlternatives:
		return [](const ProjectWithOvertime& p, const vector<int>& order) {
			return p.serialSGSWithOvertime(ActivityListPrioProvider(order));
		};
	case DecoderType::GoldenSectionSearch:
	default:
		return [](const ProjectWithOvertime& p, const vector<int>& order) {
			return p.goldenSectionSearchBasedOptimization(ActivityListPrioProvider(order));
		};
	}
}

std::string ActivityListBasedGA::selectName(DecoderType type) {
	switch(type) {
	case DecoderType::CompareAlternatives:
		return "CompareAlternativesGA";
	case DecoderType::GoldenSectionSearch:
	default:
		return "GoldenSectionSearchGA";
	}
}

ActivityListBasedGA::ActivityListBasedGA(ProjectWithOvertime & _p, DecoderType type): ActivityListBasedGA(_p, selectName(type), selectDecoder(type)) {
}

Lambda ActivityListBasedGA::init(int ix) {
    Lambda l;
    l.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    return l;
}

void ActivityListBasedGA::crossover(Lambda &mother, Lambda &father, Lambda &daughter) {
    daughter.randomOnePointCrossover(mother, father);
}

void ActivityListBasedGA::mutate(Lambda &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate, params.enforceTopOrdering);
}

FitnessResult ActivityListBasedGA::fitness(Lambda &i) {
	auto res = decoder(p, i.order);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> ActivityListBasedGA::decode(Lambda& i) {
	return decoder(p, i.order).sts;
}

//======================================================================================================================

CompareAlternativesGA::CompareAlternativesGA(ProjectWithOvertime& _p): ActivityListBasedGA(_p, DecoderType::CompareAlternatives) {
}

//======================================================================================================================

GoldenSectionSearchGA::GoldenSectionSearchGA(ProjectWithOvertime& _p): ActivityListBasedGA(_p, DecoderType::GoldenSectionSearch) {
}
