//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"
#include "Sampling.h"


LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    LambdaZrt indiv;
    indiv.order = Sampling::naiveSampling(p);
    Utils::resizeMatrix(indiv.z, p.numRes, p.numPeriods);
    EACH_RNG(r, p.numRes,
        EACH_RNG(t, p.numPeriods,
           indiv.z[r][t] = Utils::randRangeIncl(0, p.zmax[r])
        )
    )
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
    EACH_RNG(r, p.numRes, indiv.z[r] = Utils::randRangeIncl(0, p.zmax[r]))
    return indiv;
}

void FixedCapacityGA::crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) {

}

void FixedCapacityGA::mutate(LambdaZr &i) {
    neighborhoodSwap(i.order);
}

float FixedCapacityGA::fitness(LambdaZr &i) {
    auto order = p.serialSGS(i.order, i.z);
    return 0.0f;
}
