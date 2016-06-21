//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"
#include "Sampling.h"

TimeVaryingCapacityGA::TimeVaryingCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeVaryingCapacityGA") {
    useThreads = false;
}

LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    LambdaZrt indiv(p.numJobs, p.numRes, p.heuristicMakespanUpperBound());
	indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
	indiv.z.foreachAssign([this, ix](int r, int t) {
		return ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]);
	});
    return indiv;
}

void TimeVaryingCapacityGA::crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) {
    daughter.randomOnePointCrossover(mother, father);
	daughter.z.foreachAssign([&](int r, int t) { return Utils::randBool() ? mother.z(r,t) : father.z(r,t); });
}

void TimeVaryingCapacityGA::mutate(LambdaZrt &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
	mutateOvertime(i.z);
}

float TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
	auto pair = p.serialSGS(i.order, i.z);
	return p.calcProfit(pair);
}

vector<int> TimeVaryingCapacityGA::decode(LambdaZrt& i) {
	return p.serialSGS(i.order, i.z).sts;
}

void TimeVaryingCapacityGA::mutateOvertime(Matrix<int>& z) {
	z.foreach2([&](int r, int t) {
        withMutProb([&]{
            if (Utils::randBool()) z(r,t)++;
            else z(r,t)--;
            z(r,t) = z(r,t) < 0 ? 0 : (z(r,t) > p.zmax[r] ? p.zmax[r] : z(r,t));
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
    daughter.randomOnePointCrossover(mother, father);
    p.eachRes([&](int r){ daughter.z[r] = Utils::randBool() ? mother.z[r] : father.z[r]; });
}

void FixedCapacityGA::mutate(LambdaZr &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
	mutateOvertime(i.z);
}

float FixedCapacityGA::fitness(LambdaZr &i) {
	auto pair = p.serialSGS(i.order, i.z);
	return p.calcProfit(pair);
}

vector<int> FixedCapacityGA::decode(LambdaZr& i) {
	return p.serialSGS(i.order, i.z).sts;
}

void FixedCapacityGA::mutateOvertime(vector<int> &z) {
    p.eachRes([&](int r) {
        withMutProb([&]{
            if (Utils::randBool()) z[r]++;
            else z[r]--;
            z[r] = z[r] < 0 ? 0 : (z[r] > p.zmax[r] ? p.zmax[r] : z[r]);
        });
    });
}
