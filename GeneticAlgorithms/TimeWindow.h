//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_TIMEWINDOWGAS_H
#define CPP_RCPSP_OC_TIMEWINDOWGAS_H

#include "GeneticAlgorithm.h"

struct LambdaBeta {
	LambdaBeta(Project &p) : order(p.numJobs), beta(p.numJobs) {}
	LambdaBeta() {}
	vector<int> order, beta;
};
struct LambdaTau {
	LambdaTau(Project &p) : order(p.numJobs), tau(p.numJobs) {}
	LambdaTau() {}
	vector<int> order; vector<float> tau;
};

class TimeWindowBordersGA : public GeneticAlgorithm<LambdaBeta> {
protected:
public:
    TimeWindowBordersGA(ProjectWithOvertime &_p);
    virtual string getName() const override;
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
    virtual string getName() const override;
private:
    virtual LambdaTau init(int ix) override;
    virtual void crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) override;
    virtual void mutate(LambdaTau &i) override;
    virtual float fitness(LambdaTau &i) override;
	virtual vector<int> decode(LambdaTau& i) override;
};

class CompareAlternativesGA : public GeneticAlgorithm<vector<int>> {
public:
    CompareAlternativesGA(ProjectWithOvertime &_p);
    virtual string getName() const override;
private:
    virtual vector<int> init(int ix) override;
    virtual void crossover(vector<int> &mother, vector<int> &father, vector<int> &daughter) override;
    virtual void mutate(vector<int> &i) override;
    virtual float fitness(vector<int> &i) override;
	virtual vector<int> decode(vector<int>& i) override;
};


#endif //CPP_RCPSP_OC_TIMEWINDOWGAS_H
