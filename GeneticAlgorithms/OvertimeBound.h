//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H
#define CPP_RCPSP_OC_OVERTIMEBOUNDGAS_H

#include "GeneticAlgorithm.h"

struct LambdaZr {
	LambdaZr(Project &p) : order(p.numJobs), z(p.numRes) {}
	LambdaZr() {}
	vector<int> order, z;
};
struct LambdaZrt {
	LambdaZrt(Project &p) : order(p.numJobs), z(p.numRes, p.numPeriods) {}
	LambdaZrt() {}
	vector<int> order;
	Matrix<int> z;
};

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
public:
    TimeVaryingCapacityGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p) { }
private:
    virtual LambdaZrt init(int ix) override;
    virtual void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) override;
	virtual void mutate(LambdaZrt &i) override;
    virtual float fitness(LambdaZrt &i) override;
	virtual vector<int> decode(LambdaZrt& i) override;

	void mutateOvertime(Matrix<int> &z);
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
