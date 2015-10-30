//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
#define CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H

#include "GeneticAlgorithm.h"

struct LambdaZr { vector<int> order, z; };
struct LambdaZrt { vector<int> order; vector<vector<int>> z; };

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
public:
    TimeVaryingCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual LambdaZrt init(int ix);
    virtual void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter);
    virtual void mutate(LambdaZrt &i);
    virtual float fitness(LambdaZrt &i);
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
public:
    FixedCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual LambdaZr init(int ix);
    virtual void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter);
    virtual void mutate(LambdaZr &i);
    virtual float fitness(LambdaZr &i);
};


#endif //CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
