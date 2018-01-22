//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"
#include "Sampling.h"
#include <boost/algorithm/clamp.hpp>

using namespace std;

TimeVaryingCapacityGA::TimeVaryingCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeVaryingCapacityGA") {
    useThreads = false;
}

LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    LambdaZrt indiv(p.numJobs, p.numRes, p.heuristicMakespanUpperBound());
	indiv.order = params.enforceTopOrdering ? (ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p)) : Sampling::randomPermutation(p.numJobs);

	p.eachResConst([&](int r) {
		int zr = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]);
		p.eachPeriodBoundedConst([&](int t) {
			indiv.z(r, t) = zr;
		});
	});

    return indiv;
}

void TimeVaryingCapacityGA::crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) {
	switch(params.crossoverMethod) {
		case CrossoverMethod::OPC:
			daughter.randomIndependentOnePointCrossovers(mother, father, p.getHeuristicMaxMakespan());
			break;
		case CrossoverMethod::TPC:
			daughter.randomIndependentTwoPointCrossovers(mother, father, p.getHeuristicMaxMakespan());
			break;
	}
}

void TimeVaryingCapacityGA::mutate(LambdaZrt &i) {
    //i.neighborhoodSwap(p.adjMx, params.pmutate);
	//mutateOvertime(i.z);
	i.independentMutations(p.adjMx, p.zmax, params.pmutate, params.enforceTopOrdering);
}

FitnessResult TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
	// TODO: Employ parallel sgs if chosen

	const auto pair = p.serialSGSWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.z, !params.enforceTopOrdering);

	if(params.fbiFeedbackInjection) {
		i.order = p.scheduleToActivityList(pair.sts);
	}

	return { p.calcProfit(pair), pair.numSchedulesGenerated };
}

vector<int> TimeVaryingCapacityGA::decode(LambdaZrt& i) {
	// TODO: Employ parallel sgs if chosen

	return p.serialSGSWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.z, !params.enforceTopOrdering).sts;
}

void TimeVaryingCapacityGA::mutateOvertime(Matrix<int>& z) const {
	p.eachResConst([&](int r) {
		int zOffset = Utils::randBool() ? 1 : -1;
		p.eachPeriodBoundedConst([&](int t) {
			withMutProb([&] {
				z(r, t) = boost::algorithm::clamp(z(r, t) + zOffset, 0, p.zmax[r]);
			});
		});		
	});
}

//===========================================================================================================

FixedCapacityGA::FixedCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "FixedCapacityGA") {
    useThreads = false;
}

LambdaZr FixedCapacityGA::init(int ix) {
    LambdaZr indiv(p.numJobs, p.numRes);
    indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    p.eachRes([&](int r){ indiv.z[r] = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]); });
    return indiv;
}

void FixedCapacityGA::crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) {
	daughter.randomIndependentOnePointCrossovers(mother, father);
}

void FixedCapacityGA::mutate(LambdaZr &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate, params.enforceTopOrdering);
	mutateOvertime(i.z);
}

FitnessResult FixedCapacityGA::fitness(LambdaZr &i) {
	// TODO: Employ parallel sgs if chosen

	auto res = p.serialSGSWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.z);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> FixedCapacityGA::decode(LambdaZr& i) {
	// TODO: Employ parallel sgs if chosen

	return p.serialSGSWithForwardBackwardImprovement(ActivityListPrioProvider(i.order), i.z).sts;
}

void FixedCapacityGA::mutateOvertime(vector<int> &z) {
    p.eachRes([&](int r) {
        withMutProb([&]{
			z[r] = boost::algorithm::clamp(z[r] + (Utils::randBool() ? 1 : -1), 0, p.zmax[r]);
        });
    });
}

//============================================================================================

// TODO: Refactor random key codepath for reduced redundancy

TimeVaryingCapacityRandomKeyGA::TimeVaryingCapacityRandomKeyGA(ProjectWithOvertime& _p) : GeneticAlgorithm(_p, "TimeVaryingCapacityRandomKeyGA") {
	useThreads = false;
}

RandomKeyZrt TimeVaryingCapacityRandomKeyGA::init(int ix) {
	RandomKeyZrt indiv(p.numJobs, p.numRes, p.heuristicMakespanUpperBound());
	indiv.priorities = params.rbbrs ? p.activityListToRandomKey(Sampling::sample(true, p)) : Sampling::randomUnitFloats(p.numJobs);

	p.eachResConst([&](int r) {
		int zr = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]);
		p.eachPeriodBoundedConst([&](int t) {
			indiv.z(r, t) = zr;
		});
	});

	return indiv;
}

void TimeVaryingCapacityRandomKeyGA::crossover(RandomKeyZrt& mother, RandomKeyZrt& father, RandomKeyZrt& daughter) {
	daughter.randomIndependentOnePointCrossovers(mother, father, p.getHeuristicMaxMakespan());
}

void TimeVaryingCapacityRandomKeyGA::mutate(RandomKeyZrt& i) {
	i.independentMutations(p.adjMx, p.zmax, params.pmutate);
}

FitnessResult TimeVaryingCapacityRandomKeyGA::fitness(RandomKeyZrt& i) {
	auto res = p.serialSGSWithRandomKeyAndFBI(i.priorities, i.z);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

std::vector<int> TimeVaryingCapacityRandomKeyGA::decode(RandomKeyZrt& i) {
	return p.serialSGSWithRandomKeyAndFBI(i.priorities, i.z).sts;
}

//============================================================================================

FixedCapacityRandomKeyGA::FixedCapacityRandomKeyGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "FixedCapacityRandomKeyGA") {
	useThreads = false;
}

RandomKeyZr FixedCapacityRandomKeyGA::init(int ix) {
	RandomKeyZr indiv(p.numJobs, p.numRes);
	indiv.priorities = params.rbbrs ? p.activityListToRandomKey(Sampling::sample(true, p)) : Sampling::randomUnitFloats(p.numJobs);
	p.eachRes([&](int r) { indiv.z[r] = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]); });
	return indiv;
}

void FixedCapacityRandomKeyGA::crossover(RandomKeyZr &mother, RandomKeyZr &father, RandomKeyZr &daughter) {
	daughter.randomIndependentOnePointCrossovers(mother, father);
}

void FixedCapacityRandomKeyGA::mutate(RandomKeyZr &i) {
	i.mutate(params.pmutate);
	mutateOvertime(i.z);
}

FitnessResult FixedCapacityRandomKeyGA::fitness(RandomKeyZr &i) {
	auto res = p.serialSGSWithRandomKeyAndFBI(i.priorities, i.z);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> FixedCapacityRandomKeyGA::decode(RandomKeyZr& i) {
	return p.serialSGSWithRandomKeyAndFBI(i.priorities, i.z).sts;
}

void FixedCapacityRandomKeyGA::mutateOvertime(vector<int> &z) {
	p.eachRes([&](int r) {
		withMutProb([&] {
			z[r] = boost::algorithm::clamp(z[r] + (Utils::randBool() ? 1 : -1), 0, p.zmax[r]);
		});
	});
}