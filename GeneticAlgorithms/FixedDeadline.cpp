//
// Created by André Schnabel on 30.10.15.
//

#include <cmath>
#include "FixedDeadline.h"

FixedDeadlineGA::FixedDeadlineGA(ProjectWithOvertime &_p): GeneticAlgorithm(_p, "FixedDeadlineGA") {
    useThreads = true;
    deadlineUB = p.makespan(p.serialSGS(p.topOrder));
    deadlineLB = p.makespan(p.serialSGS(p.topOrder, p.zmax).sts);
	fallbackSts = p.serialSGS(p.topOrder);
}

DeadlineLambda FixedDeadlineGA::init(int ix) {
    DeadlineLambda indiv(p.numJobs);
	indiv.deadline = (ix == 0) ? p.numPeriods - 1 : Utils::randRangeIncl(deadlineLB, deadlineUB);
    indiv.order = p.topOrder;
	if(ix > 0) random_shuffle(indiv.order.begin(), indiv.order.end());
    return indiv;
}

void FixedDeadlineGA::crossover(DeadlineLambda &mother, DeadlineLambda &father, DeadlineLambda &daughter) {
    mother.randomOnePointCrossover(mother, father);
}

void FixedDeadlineGA::mutate(DeadlineLambda &i) {
    for(int ix=1; ix<p.numJobs; ix++)
        withMutProb([&] { Utils::swap(i.order, ix-1, ix); });
    withMutProb([&] { i.deadline = Utils::randBool() ? max(deadlineLB, i.deadline-1) : min(deadlineUB, i.deadline+1); });
}

float FixedDeadlineGA::fitness(DeadlineLambda &i) {
    auto res = p.serialSGSWithDeadlineEarly(i.deadline, i.order);
	if (res.valid) return p.calcProfit(res);
	return 0.0f;
}

vector<int> FixedDeadlineGA::decode(DeadlineLambda &i) {
	auto res = p.serialSGSWithDeadlineEarly(i.deadline, i.order);
	if (res.valid) return res.sts;
	return fallbackSts;
}
