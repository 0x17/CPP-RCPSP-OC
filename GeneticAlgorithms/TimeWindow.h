//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_TIMEWINDOWGAS_H
#define CPP_RCPSP_OC_TIMEWINDOWGAS_H

#include "GeneticAlgorithm.h"
#include "Representations.h"

class TimeWindowBordersGA : public GeneticAlgorithm<LambdaBeta> {
	static ProjectWithOvertime::BorderSchedulingOptions options;
public:
	static void setVariant(int variant);

	TimeWindowBordersGA(ProjectWithOvertime &_p);
protected:
    virtual LambdaBeta init(int ix) override;
    virtual void crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) override;
    virtual void mutate(LambdaBeta &i) override;
    virtual float fitness(LambdaBeta &i) override;
	virtual vector<int> decode(LambdaBeta& i) override;
};

class TimeWindowArbitraryGA : public GeneticAlgorithm<LambdaTau> {
public:
    TimeWindowArbitraryGA(ProjectWithOvertime &_p);
private:
    virtual LambdaTau init(int ix) override;
    virtual void crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) override;
    virtual void mutate(LambdaTau &i) override;
    virtual float fitness(LambdaTau &i) override;
	virtual vector<int> decode(LambdaTau& i) override;
};

class CompareAlternativesGA : public GeneticAlgorithm<Lambda> {
public:
    CompareAlternativesGA(ProjectWithOvertime &_p);
private:
    virtual Lambda init(int ix) override;
    virtual void crossover(Lambda &mother, Lambda &father, Lambda &daughter) override;
    virtual void mutate(Lambda &i) override;
    virtual float fitness(Lambda &i) override;
	virtual vector<int> decode(Lambda& i) override;
};


#endif //CPP_RCPSP_OC_TIMEWINDOWGAS_H
