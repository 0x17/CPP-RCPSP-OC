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
    virtual LambdaZrt init(int ix) override;
    virtual void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) override;
    virtual void mutate(LambdaZrt &i) override;
    virtual float fitness(LambdaZrt &i) override;
	virtual vector<int> decode(LambdaZrt& i) override;
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
public:
    FixedCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual LambdaZr init(int ix) override;
	virtual void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) override;	
	virtual void mutate(LambdaZr &i) override;
    virtual float fitness(LambdaZr &i) override;
	virtual vector<int> decode(LambdaZr& i) override;

	void mutateOvertime(vector<int> &z);
};


#endif //CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
