//
// Created by André Schnabel on 30.10.15.
//

#pragma once

#include "GeneticAlgorithm.h"
#include "Representations.h"

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
public:
    TimeVaryingCapacityGA(ProjectWithOvertime &_p);
private:
    virtual LambdaZrt init(int ix) override;
    virtual void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) override;
	virtual void mutate(LambdaZrt &i) override;
    virtual FitnessResult fitness(LambdaZrt &i) override;
	virtual std::vector<int> decode(LambdaZrt& i) override;

	void mutateOvertime(Matrix<int> &z) const;
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
public:
    FixedCapacityGA(ProjectWithOvertime &_p);
private:
    virtual LambdaZr init(int ix) override;
	virtual void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) override;	
	virtual void mutate(LambdaZr &i) override;
    virtual FitnessResult fitness(LambdaZr &i) override;
	virtual std::vector<int> decode(LambdaZr& i) override;

	void mutateOvertime(std::vector<int> &z);
};
