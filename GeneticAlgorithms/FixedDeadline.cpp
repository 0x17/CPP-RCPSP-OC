//
// Created by André Schnabel on 30.10.15.
//

#include <cmath>
#include "FixedDeadline.h"

FixedDeadlineGA::FixedDeadlineGA(ProjectWithOvertime &_p): GeneticAlgorithm(_p, "FixedDeadlineGA") {
    useThreads = true;
    deadlineUB = p.makespan(p.serialSGS(p.topOrder));
    deadlineLB = p.makespan(p.serialSGS(p.topOrder, p.zmax).first);
}

DeadlineLambda FixedDeadlineGA::init(int ix) {
    DeadlineLambda indiv(p.numJobs);
    indiv.deadline = Utils::randRangeIncl(deadlineLB, deadlineUB);
    indiv.order = p.topOrder;
    random_shuffle(indiv.order.begin(), indiv.order.end());
    return indiv;
}

void FixedDeadlineGA::crossover(DeadlineLambda &mother, DeadlineLambda &father, DeadlineLambda &daughter) {
    mother.randomOnePointCrossover(mother, father);
}

void FixedDeadlineGA::mutate(DeadlineLambda &i) {
    for(int ix=1; ix<p.numJobs; ix++)
        withMutProb([&] {
            int tmp = i.order[ix-1];
            i.order[ix-1] = i.order[ix];
            i.order[ix] = tmp;
        });
    withMutProb([&] { i.deadline = (rand() % 2 == 0) ? max(deadlineLB, i.deadline-1) : min(deadlineUB, i.deadline+1); });
}

float FixedDeadlineGA::fitness(DeadlineLambda &i) {
    auto res = p.serialSGSWithDeadline(i.deadline, i.order);
	if (res.first) return profitForSGSResult(res.second);
	return numeric_limits<float>::lowest();
}

vector<int> FixedDeadlineGA::decode(DeadlineLambda &i) {
	auto res = p.serialSGSWithDeadline(i.deadline, i.order);
	if(res.first) return res.second.first;	
	vector<int> sts(p.numJobs, -1);
	return sts;
}
