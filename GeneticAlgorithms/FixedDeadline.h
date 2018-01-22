//
// Created by André Schnabel on 30.10.15.
//

#pragma once

#include "GeneticAlgorithm.h"
#include "Representations.h"

class FixedDeadlineGA : public GeneticAlgorithm<DeadlineLambda> {
public:
    FixedDeadlineGA(ProjectWithOvertime &_p);

private:
    virtual DeadlineLambda init(int ix) override;
    virtual void crossover(DeadlineLambda &mother, DeadlineLambda &father, DeadlineLambda &daughter) override;
    virtual void mutate(DeadlineLambda &i) override;
    virtual FitnessResult fitness(DeadlineLambda &i) override;
    virtual std::vector<int> decode(DeadlineLambda &i) override;

    int deadlineOffsetLB, deadlineOffsetUB;
};
