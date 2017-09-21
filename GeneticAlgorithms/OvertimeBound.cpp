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
	indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);

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
	i.independentMutations(p.adjMx, p.zmax, params.pmutate);
}

FitnessResult TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
	auto pair = p.serialSGSWithForwardBackwardImprovement(i.order, i.z);
	return { p.calcProfit(pair), pair.numSchedulesGenerated };
}

vector<int> TimeVaryingCapacityGA::decode(LambdaZrt& i) {
	return p.serialSGSWithForwardBackwardImprovement(i.order, i.z).sts;
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
    i.neighborhoodSwap(p.adjMx, params.pmutate);
	mutateOvertime(i.z);
}

FitnessResult FixedCapacityGA::fitness(LambdaZr &i) {
	auto res = p.serialSGSWithForwardBackwardImprovement(i.order, i.z);
	return { p.calcProfit(res), res.numSchedulesGenerated };
}

vector<int> FixedCapacityGA::decode(LambdaZr& i) {
	return p.serialSGSWithForwardBackwardImprovement(i.order, i.z).sts;
}

void FixedCapacityGA::mutateOvertime(vector<int> &z) {
    p.eachRes([&](int r) {
        withMutProb([&]{
			z[r] = boost::algorithm::clamp(z[r] + (Utils::randBool() ? 1 : -1), 0, p.zmax[r]);
        });
    });
}
