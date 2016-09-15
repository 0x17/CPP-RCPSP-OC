//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
#define CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H

#include "GeneticAlgorithm.h"
#include "Representations.h"

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
public:
    TimeVaryingCapacityGA(ProjectWithOvertime &_p);
private:
    virtual LambdaZrt init(int ix) override;
    virtual void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) override;
	virtual void mutate(LambdaZrt &i) override;
    virtual float fitness(LambdaZrt &i) override;
	virtual vector<int> decode(LambdaZrt& i) override;

	void mutateOvertime(Matrix<int> &z) const;
	void crossoverOvertime(Matrix<int>& daughterZ, const Matrix<int>& motherZ, const Matrix<int>& fatherZ) const;
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
public:
    FixedCapacityGA(ProjectWithOvertime &_p);
private:
    virtual LambdaZr init(int ix) override;
	virtual void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) override;	
	virtual void mutate(LambdaZr &i) override;
    virtual float fitness(LambdaZr &i) override;
	virtual vector<int> decode(LambdaZr& i) override;

	void mutateOvertime(vector<int> &z);
};


#endif //CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
