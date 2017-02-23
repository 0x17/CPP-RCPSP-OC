//
// Created by André Schnabel on 06.02.16.
//

#include <cmath>
#include "Representations.h"
#include <boost/algorithm/clamp.hpp>

Lambda::Lambda(int numJobs) : order(numJobs) {}

Lambda::Lambda(const vector<int> &_order) : order(_order) {}

Lambda::Lambda() {}

void Lambda::neighborhoodSwap(const Matrix<char> &adjMx, int pmutate) {
    for(int i=1; i<order.size(); i++) {
        if(Utils::randRangeIncl(1, 100) <= pmutate && !adjMx(order[i - 1], order[i])) {
            swap(i-1, i);
        }
    }
}

void Lambda::randomOnePointCrossover(const Lambda &mother, const Lambda& father) {
    int q = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
    onePointCrossover(mother, father, q);
}

void Lambda::onePointCrossover(const Lambda &mother, const Lambda& father, int q) {
    for(int i = 0; i <= q; i++) { inherit(mother, i, i); }

    for(int i = 0, ctr = q + 1; i<order.size(); i++) {
        if(!Utils::rangeInclContains(order, 0, q, father.order[i])) {
            inherit(father, ctr, i);
            ctr++;
        }
    }
}

void Lambda::inherit(const Lambda &parent, int destIx, int srcIx) {
    order[destIx] = parent.order[srcIx];
}

void Lambda::randomTwoPointCrossover(const Lambda &mother, const Lambda &father) {
    int q1 = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 2);
    int q2 = Utils::randRangeIncl(q1 + 1, static_cast<int>(order.size()) - 1);
    twoPointCrossover(mother, father, q1, q2);
}

void Lambda::twoPointCrossover(const Lambda &mother, const Lambda &father, int q1, int q2) {
    int len = static_cast<int>(order.size());

    for (int i = 0, ctr = 0; i <= q1; i++, ctr++) {
        inherit(mother, ctr, i);
    }

    for (int i = 0, ctr = q1 + 1; i < len && ctr <= q2; i++) {
        if (!Utils::rangeInclContains(order, 0, q1, father.order[i])) {
            inherit(father, ctr, i);
            ctr++;
        }
    }

    for (int i = q1 + 1, ctr = q2 + 1; i < len && ctr < len; i++) {
        if (!Utils::rangeInclContains(order, q1 + 1, q2, mother.order[i])) {
            inherit(mother, ctr, i);
            ctr++;
        }
    }
}

void Lambda::swap(int i1, int i2) {
    Utils::swap(order, i1, i2);
}

//======================================================================================================================

DeadlineLambda::DeadlineLambda(int numJobs) : Lambda(numJobs), deadlineOffset(0) {}

DeadlineLambda::DeadlineLambda(): deadlineOffset(0) {}

void DeadlineLambda::randomOnePointCrossover(const Lambda &mother, const Lambda &father) {
    Lambda::randomOnePointCrossover(mother, father);
    auto &m = static_cast<const DeadlineLambda&>(mother);
    auto &f = static_cast<const DeadlineLambda&>(father);
    deadlineOffset = static_cast<int>(std::round(static_cast<float>(m.deadlineOffset - f.deadlineOffset) / 2.0f)) + f.deadlineOffset;
}

//======================================================================================================================

LambdaZr::LambdaZr(int numJobs, int numRes) : Lambda(numJobs), z(numRes) {}

LambdaZr::LambdaZr() {}

void LambdaZr::randomIndependentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father) {
	int qj = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
	int qr = Utils::randRangeIncl(0, static_cast<int>(z.size()) - 1);
	independentOnePointCrossovers(mother, father, qj, qr);
}

void LambdaZr::independentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father, int qj, int qr) {
	onePointCrossover(mother, father, qj);
	for(int r = 0; r < z.size(); r++)
		z[r] = r <= qr ? mother.z[r] : father.z[r];
}

//======================================================================================================================

LambdaZrt::LambdaZrt(int numJobs, int numRes, int numPeriods) : Lambda(numJobs), z(numRes, numPeriods) {}

LambdaZrt::LambdaZrt() {}

void LambdaZrt::randomIndependentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan) {
	CrossoverPartitionType ctype = Utils::randBool() ? CrossoverPartitionType::PERIOD_WISE : CrossoverPartitionType::RESOURCE_WISE;
	int qj = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
	int q2 = Utils::randRangeIncl(0, (ctype == CrossoverPartitionType::PERIOD_WISE ? heuristicMaxMakespan : z.getM()) - 1);
	independentOnePointCrossovers(mother, father, qj, q2, ctype);
}

void LambdaZrt::independentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int qj, int q2, CrossoverPartitionType ctype) {
	onePointCrossover(mother, father, qj);
	switch(ctype) {
	default:
	case CrossoverPartitionType::RESOURCE_WISE:
		z.foreachAssign([&](int r, int t) {
			return r <= q2 ? mother.z(r, t) : father.z(r, t);
		});
		break;
	case CrossoverPartitionType::PERIOD_WISE:
		z.foreachAssign([&](int r, int t) {
			return t <= q2 ? mother.z(r, t) : father.z(r, t);
		});
		break;
	}
}

void LambdaZrt::independentMutations(const Matrix<char>& adjMx, const vector<int> &zmax, int pmutate) {
	neighborhoodSwap(adjMx, pmutate);
	for(int r = 0; r < z.getM(); r++) {
		int zOffset = Utils::randBool() ? 1 : -1;
		for(int t = 0; t < z.getN(); t++) {
			if (Utils::randRangeIncl(1, 100) <= pmutate) {
				z(r, t) = boost::algorithm::clamp(z(r, t) + zOffset, 0, zmax[r]);
			}
		}
	}
}

//======================================================================================================================

LambdaBeta::LambdaBeta(int numJobs) : Lambda(numJobs), beta(numJobs) {}

void LambdaBeta::inherit(const Lambda &parent, int destIx, int srcIx) {
    Lambda::inherit(parent, destIx, srcIx);
    beta[destIx] = static_cast<const LambdaBeta &>(parent).beta[srcIx];
}

void LambdaBeta::swap(int i1, int i2) {
    Lambda::swap(i1, i2);
	if(!options.assocIndex)
		Utils::swap(beta, i1, i2);
}

void LambdaBeta::separateOnePointCrossover(const LambdaBeta& mother, const LambdaBeta& father) {
	int q1 = Utils::randRangeIncl(0, static_cast<int>(order.size() - 1));
	int q2 = Utils::randRangeIncl(0, static_cast<int>(beta.size() - 1));
	onePointCrossoverLists(q1, order, mother.order, father.order);
	onePointCrossoverLists(q2, beta, mother.beta, father.beta);
}

ProjectWithOvertime::BorderSchedulingOptions LambdaBeta::options;

void LambdaBeta::setOptions(ProjectWithOvertime::BorderSchedulingOptions _options) {
	options = _options;
}

//======================================================================================================================

LambdaTau::LambdaTau(int numJobs) : Lambda(numJobs), tau(numJobs) {}

LambdaTau::LambdaTau() {}

void LambdaTau::inherit(const Lambda &parent, int destIx, int srcIx) {
    Lambda::inherit(parent, destIx, srcIx);
    tau[destIx] = static_cast<const LambdaTau &>(parent).tau[srcIx];
}

void LambdaTau::swap(int i1, int i2) {
    Lambda::swap(i1, i2);
    Utils::swap(tau, i1, i2);
}
