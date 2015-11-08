//
// Created by André Schnabel on 30.10.15.
//

#include <cmath>
#include "FixedDeadline.h"

FixedDeadlineGA::FixedDeadlineGA(ProjectWithOvertime &_p): GeneticAlgorithm(_p) {
    useThreads = true;
    deadlineUB = p.makespan(p.serialSGS(p.topOrder));
    deadlineLB = p.makespan(p.serialSGS(p.topOrder, p.zmax).first);
}

DeadlineLambda FixedDeadlineGA::init(int ix) {
    DeadlineLambda indiv(p);
    indiv.deadline = Utils::randRangeIncl(deadlineLB, deadlineUB);
    indiv.order = p.topOrder;
    random_shuffle(indiv.order.begin(), indiv.order.end());
    return indiv;
}

void FixedDeadlineGA::crossover(DeadlineLambda &mother, DeadlineLambda &father, DeadlineLambda &daughter) {
    onePointCrossover({mother.order, father.order, daughter.order});
    daughter.deadline = static_cast<int>(std::round(static_cast<float>(mother.deadline - father.deadline) / 2.0f)) + father.deadline;
}

void FixedDeadlineGA::mutate(DeadlineLambda &i) {
    for(int ix=1; ix<p.numJobs; ix++)
        withMutProb([&] { swap(i.order, ix-1, ix); });
    withMutProb([&] { i.deadline = (rand() % 2 == 0) ? max(deadlineLB, i.deadline-1) : min(deadlineUB, i.deadline+1); });
}

float FixedDeadlineGA::fitness(DeadlineLambda &i) {
    auto pair = p.serialSGSWithDeadline(i.deadline, i.order);
    return profitForSGSResult(pair);
}

vector<int> FixedDeadlineGA::decode(DeadlineLambda &i) {
    return p.serialSGSWithDeadline(i.deadline, i.order).first;
}
