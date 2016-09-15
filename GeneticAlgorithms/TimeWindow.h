//
// Created by André Schnabel on 30.10.15.
//

#ifndef CPP_RCPSP_OC_TIMEWINDOWGAS_H
#define CPP_RCPSP_OC_TIMEWINDOWGAS_H

#include <map>

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

class TimeWindowArbitraryDiscretizedGA : public GeneticAlgorithm<LambdaBeta> {
public:
	TimeWindowArbitraryDiscretizedGA(ProjectWithOvertime &_p, int _ub = 4);
private:
	virtual LambdaBeta init(int ix) override;
	virtual void crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) override;
	virtual void mutate(LambdaBeta &i) override;
	virtual float fitness(LambdaBeta &i) override;
	virtual vector<int> decode(LambdaBeta& i) override;

	int ub;
};

class ActivityListBasedGA : public GeneticAlgorithm<Lambda> {
public:	
	using TDecoder = function<SGSResult(const ProjectWithOvertime &p, const vector<int>&)>;

	enum class DecoderType {
		CompareAlternatives,
		GoldenCutSearch
	};

	ActivityListBasedGA(ProjectWithOvertime &_p, string name, TDecoder _decoder);
	ActivityListBasedGA(ProjectWithOvertime& _p, DecoderType type);
	
	static TDecoder selectDecoder(DecoderType type);
	static string selectName(DecoderType type);

private:
	TDecoder decoder;

    virtual Lambda init(int ix) override;
    virtual void crossover(Lambda &mother, Lambda &father, Lambda &daughter) override;
    virtual void mutate(Lambda &i) override;
    virtual float fitness(Lambda &i) override;
	virtual vector<int> decode(Lambda& i) override;
};

class CompareAlternativesGA : public ActivityListBasedGA {
public:
	CompareAlternativesGA(ProjectWithOvertime& _p);
};

class GoldenCutSearchGA : public ActivityListBasedGA {
public:
	GoldenCutSearchGA(ProjectWithOvertime& _p);
};


#endif //CPP_RCPSP_OC_TIMEWINDOWGAS_H
