//
// Created by André Schnabel on 30.10.15.
//

#pragma once

#include "GeneticAlgorithm.h"
#include "Representations.h"

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
public:
    explicit TimeVaryingCapacityGA(ProjectWithOvertime &_p);
private:
	LambdaZrt init(int ix) override;
	void crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) override;
	void mutate(LambdaZrt &i) override;
    FitnessResult fitness(LambdaZrt &i) override;
	std::vector<int> decode(LambdaZrt& i) override;

	void mutateOvertime(Matrix<int> &z) const;
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
public:
    explicit FixedCapacityGA(ProjectWithOvertime &_p);
private:
    LambdaZr init(int ix) override;
	void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) override;
	void mutate(LambdaZr &i) override;
    FitnessResult fitness(LambdaZr &i) override;
	std::vector<int> decode(LambdaZr& i) override;

	void mutateOvertime(std::vector<int> &z);
};

// TODO: Refactor random key codepath for reduced redundancy

class TimeVaryingCapacityRandomKeyGA : public GeneticAlgorithm<RandomKeyZrt> {
public:
	explicit TimeVaryingCapacityRandomKeyGA(ProjectWithOvertime &_p);
private:
	RandomKeyZrt init(int ix) override;
	void crossover(RandomKeyZrt &mother, RandomKeyZrt &father, RandomKeyZrt &daughter) override;
	void mutate(RandomKeyZrt &i) override;
	FitnessResult fitness(RandomKeyZrt &i) override;
	std::vector<int> decode(RandomKeyZrt& i) override;
};