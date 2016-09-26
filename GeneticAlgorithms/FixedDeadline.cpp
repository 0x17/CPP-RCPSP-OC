//
// Created by André Schnabel on 30.10.15.
//

#include <cmath>
#include "FixedDeadline.h"
#include "Sampling.h"

FixedDeadlineGA::FixedDeadlineGA(ProjectWithOvertime &_p): GeneticAlgorithm(_p, "FixedDeadlineGA") {
    useThreads = true;
    deadlineOffsetUB = p.makespan(p.serialSGS(p.topOrder)) - p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
    deadlineOffsetLB = 0;
}

DeadlineLambda FixedDeadlineGA::init(int ix) {
    DeadlineLambda indiv(p.numJobs);
	indiv.deadlineOffset = (ix == 0) ? deadlineOffsetUB : Utils::randRangeIncl(deadlineOffsetLB, deadlineOffsetUB);
    indiv.order = ix == 0 ? p.topOrder : Sampling::sample(params.rbbrs, p);
    return indiv;
}

void FixedDeadlineGA::crossover(DeadlineLambda &mother, DeadlineLambda &father, DeadlineLambda &daughter) {
    mother.randomOnePointCrossover(mother, father);
}

void FixedDeadlineGA::mutate(DeadlineLambda &i) {
    i.neighborhoodSwap(p.adjMx, params.pmutate);
    withMutProb([&] { i.deadlineOffset = Utils::randBool() ? max(deadlineOffsetLB, i.deadlineOffset-1) : min(deadlineOffsetUB, i.deadlineOffset+1); });
}

float FixedDeadlineGA::fitness(DeadlineLambda &i) {
    auto res = p.forwardBackwardDeadlineOffsetSGS(i.order, i.deadlineOffset);
    return p.calcProfit(res);
}

vector<int> FixedDeadlineGA::decode(DeadlineLambda &i) {
    return p.forwardBackwardDeadlineOffsetSGS(i.order, i.deadlineOffset).sts;
}
