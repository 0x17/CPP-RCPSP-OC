//
// Created by André Schnabel on 25.01.18.
//

#pragma once

#include "Representations.h"
#include "GeneticAlgorithm.h"

class PartitionListGA : public GeneticAlgorithm<PartitionList> {
public:
	PartitionListGA(ProjectWithOvertime &_p);

protected:
	PartitionList init(int ix) override;

	void crossover(PartitionList &mother, PartitionList &father, PartitionList &daughter) override;

	void mutate(PartitionList &i) override;

	FitnessResult fitness(PartitionList &i) override;

	std::vector<int> decode(PartitionList &i) override;

};