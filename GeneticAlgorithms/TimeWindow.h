//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_TIMEWINDOWGAS_H
#define CPP_RCPSP_OC_TIMEWINDOWGAS_H

#include "GeneticAlgorithm.h"

struct LambdaBeta { vector<int> order, beta; };
struct LambdaTau { vector<int> order; vector<float> tau; };

class TimeWindowBordersGA : public GeneticAlgorithm<LambdaBeta> {
protected:
public:
    TimeWindowBordersGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
protected:
    virtual LambdaBeta init(int ix);
    virtual void crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter);
    virtual void mutate(LambdaBeta &i);
    virtual float fitness(LambdaBeta &i);
};

class TimeWindowArbitraryGA : public GeneticAlgorithm<LambdaTau> {
public:
    TimeWindowArbitraryGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual LambdaTau init(int ix);
    virtual void crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter);
    virtual void mutate(LambdaTau &i);
    virtual float fitness(LambdaTau &i);
};

class CompareAlternativesGA : public GeneticAlgorithm<vector<int>> {
public:
    CompareAlternativesGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual vector<int> init(int ix);
    virtual void crossover(vector<int> &mother, vector<int> &father, vector<int> &daughter);
    virtual void mutate(vector<int> &i);
    virtual float fitness(vector<int> &i);
};


#endif //CPP_RCPSP_OC_TIMEWINDOWGAS_H
