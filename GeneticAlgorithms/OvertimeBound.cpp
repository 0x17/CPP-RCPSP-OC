//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"
#include "Sampling.h"

LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    LambdaZrt indiv;
    indiv.order = Sampling::naiveSampling(p);
    Utils::resizeMatrix(indiv.z, p.numRes, p.numPeriods);
    P_EACH_RES(P_EACH_PERIOD(indiv.z[r][t] = Utils::randRangeIncl(0, p.zmax[r])))
    return indiv;
}

void TimeVaryingCapacityGA::crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) {
    onePointCrossover(mother.order, father.order, daughter.order);
}

void TimeVaryingCapacityGA::mutate(LambdaZrt &i) {
    neighborhoodSwap(i.order);
}

float TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
    return 0;
}

LambdaZr FixedCapacityGA::init(int ix) {
    LambdaZr indiv;
    indiv.order = Sampling::naiveSampling(p);
    indiv.z.resize(p.numRes);
    P_EACH_RES(indiv.z[r] = Utils::randRangeIncl(0, p.zmax[r]))
    return indiv;
}

void FixedCapacityGA::crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) {
	onePointCrossover(mother.order, father.order, daughter.order);
	P_EACH_RES(daughter.z[r] = rand() % 2 == 0 ? mother.z[r] : father.z[r])
}

void FixedCapacityGA::mutate(LambdaZr &i) {
    neighborhoodSwap(i.order);
	mutateOvertime(i.z);
}

float FixedCapacityGA::fitness(LambdaZr &i) {
    auto order = p.serialSGS(i.order, i.z);
    return 0.0f;
}

void FixedCapacityGA::mutateOvertime(vector<int> &z) {
	P_EACH_RES(
		int q = Utils::randRangeIncl(1, 100);
		if(q <= pmutate) {
			if (rand() % 2 == 0) z[r]++;
			else z[r]--;
			z[r] = z[r] < 0 ? 0 : (z[r] > p.zmax[r] ? p.zmax[r] : z[r]);
		}
	)
}
