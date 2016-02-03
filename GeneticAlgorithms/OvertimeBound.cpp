//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"
#include "Sampling.h"
#include "GeneticOperators.h"

TimeVaryingCapacityGA::TimeVaryingCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "TimeVaryingCapacityGA") {
    useThreads = false;
}

LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    LambdaZrt indiv(p);
    indiv.order = ix == 0 ? p.topOrder : Sampling::naiveSampling(p);
    p.eachResPeriod([&](int r, int t){ indiv.z(r,t) = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]); });
    return indiv;
}

void TimeVaryingCapacityGA::crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) {
	GeneticOperators::onePointCrossover({ mother.order, father.order, daughter.order });
    p.eachResPeriod([&](int r, int t) { daughter.z(r,t) = rand() % 2 == 0 ? mother.z(r,t) : father.z(r,t); });
}

void TimeVaryingCapacityGA::mutate(LambdaZrt &i) {
	GeneticOperators::neighborhoodSwap(p.adjMx, i.order, params.pmutate);
	mutateOvertime(i.z);
}

float TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
	auto pair = p.serialSGS(i.order, i.z);
	p.eachResPeriod([&](int r, int t) { pair.second(r,t) -= i.z(r,t); });
	return profitForSGSResult(pair);
}

vector<int> TimeVaryingCapacityGA::decode(LambdaZrt& i) {
	return p.serialSGS(i.order, i.z).first;
}

void TimeVaryingCapacityGA::mutateOvertime(Matrix<int>& z) {
    p.eachResPeriod([&](int r, int t) {
        withMutProb([&]{
            if (rand() % 2 == 0) z(r,t)++;
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
    LambdaZr indiv(p);
    indiv.order = ix == 0 ? p.topOrder : Sampling::naiveSampling(p);
    p.eachRes([&](int r){ indiv.z[r] = ix == 0 ? 0 : Utils::randRangeIncl(0, p.zmax[r]); });
    return indiv;
}

void FixedCapacityGA::crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) {
	GeneticOperators::onePointCrossover({ mother.order, father.order, daughter.order });
    p.eachRes([&](int r){ daughter.z[r] = rand() % 2 == 0 ? mother.z[r] : father.z[r]; });
}

void FixedCapacityGA::mutate(LambdaZr &i) {
	GeneticOperators::neighborhoodSwap(p.adjMx, i.order, params.pmutate);
	mutateOvertime(i.z);
}

float FixedCapacityGA::fitness(LambdaZr &i) {
	auto pair = p.serialSGS(i.order, i.z);
	p.eachResPeriod([&](int r, int t) {pair.second(r,t) -= i.z[r]; });
	return profitForSGSResult(pair);
}

vector<int> FixedCapacityGA::decode(LambdaZr& i) {
	return p.serialSGS(i.order, i.z).first;
}

void FixedCapacityGA::mutateOvertime(vector<int> &z) {
    p.eachRes([&](int r) {
        withMutProb([&]{
            if (rand() % 2 == 0) z[r]++;
            else z[r]--;
            z[r] = z[r] < 0 ? 0 : (z[r] > p.zmax[r] ? p.zmax[r] : z[r]);
        });
    });
}
