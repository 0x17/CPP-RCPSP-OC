//
// Created by André Schnabel on 25.01.18.
//

#include "PartitionList.h"
#include "Sampling.h"

using namespace std;

PartitionListGA::PartitionListGA(ProjectWithOvertime &_p) : GeneticAlgorithm(_p, "PartitionListGA") {
	useThreads = false;
}

PartitionList PartitionListGA::init(int ix) {
	PartitionList indiv(p.numJobs);
	indiv.plist = p.orderInducedPartitionsToPartitionList(Sampling::sample(params.rbbrs, p), params.partitionSize);
	return indiv;
}

void PartitionListGA::crossover(PartitionList &mother, PartitionList &father, PartitionList &daughter) {
	daughter.combine(mother, father, params.partitionSize);
}

void PartitionListGA::mutate(PartitionList &i) {
	i.partitionSwap(p.adjMx, params.pmutate, params.partitionSize);
}

FitnessResult PartitionListGA::fitness(PartitionList &i) {
	//assert(p.isPartitionListFeasible(i.plist, params.partitionSize));
	return { p.calcProfit(p.serialOptimalSubSGSWithPartitionListAndFBI(i.plist)), 1 };
}

std::vector<int> PartitionListGA::decode(PartitionList &i) {
	return p.serialOptimalSubSGSWithPartitionListAndFBI(i.plist).sts;
}
