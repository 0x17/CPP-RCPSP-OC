//
// Created by André Schnabel on 30.10.15.
//

#pragma once

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
    virtual FitnessResult fitness(LambdaBeta &i) override;
	virtual std::vector<int> decode(LambdaBeta& i) override;
};

class TimeWindowArbitraryGA : public GeneticAlgorithm<LambdaTau> {
public:
    TimeWindowArbitraryGA(ProjectWithOvertime &_p);
private:
    virtual LambdaTau init(int ix) override;
    virtual void crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) override;
    virtual void mutate(LambdaTau &i) override;
    virtual FitnessResult fitness(LambdaTau &i) override;
	virtual std::vector<int> decode(LambdaTau& i) override;
};

class TimeWindowArbitraryDiscretizedGA : public GeneticAlgorithm<LambdaBeta> {
public:
	TimeWindowArbitraryDiscretizedGA(ProjectWithOvertime &_p, int _ub = 4);
private:
	virtual LambdaBeta init(int ix) override;
	virtual void crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) override;
	virtual void mutate(LambdaBeta &i) override;
	virtual FitnessResult fitness(LambdaBeta &i) override;
	virtual std::vector<int> decode(LambdaBeta& i) override;

	int ub;
};

class ActivityListBasedGA : public GeneticAlgorithm<Lambda> {
public:	
	using TDecoder = std::function<SGSResult(const ProjectWithOvertime &p, const std::vector<int>&)>;

	enum class DecoderType {
		CompareAlternatives,
		GoldenSectionSearch
	};

	ActivityListBasedGA(ProjectWithOvertime &_p, const std::string &name, TDecoder _decoder);
	ActivityListBasedGA(ProjectWithOvertime &_p, DecoderType type);
	
	static TDecoder selectDecoder(DecoderType type);
	static std::string selectName(DecoderType type);

private:
	TDecoder decoder;

    virtual Lambda init(int ix) override;
    virtual void crossover(Lambda &mother, Lambda &father, Lambda &daughter) override;
    virtual void mutate(Lambda &i) override;
    virtual FitnessResult fitness(Lambda &i) override;
	virtual std::vector<int> decode(Lambda& i) override;
};

class CompareAlternativesGA : public ActivityListBasedGA {
public:
	CompareAlternativesGA(ProjectWithOvertime& _p);
};

class GoldenSectionSearchGA : public ActivityListBasedGA {
public:
	GoldenSectionSearchGA(ProjectWithOvertime& _p);
};
