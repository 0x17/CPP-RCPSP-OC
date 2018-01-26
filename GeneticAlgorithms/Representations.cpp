//
// Created by André Schnabel on 06.02.16.
//

#include <cmath>
#include <boost/algorithm/clamp.hpp>
#include "Representations.h"

using namespace std;

Lambda::Lambda(int numJobs) : order(numJobs) {}

Lambda::Lambda(const vector<int> &_order) : order(_order) {}

Lambda::Lambda() = default;

void Lambda::neighborhoodSwap(const Matrix<char> &adjMx, int pmutate, bool keepTopOrder) {
    for(int i=1; i<order.size(); i++) {
        if(Utils::randRangeIncl(1, 100) <= pmutate && (!keepTopOrder || !adjMx(order[i - 1], order[i]))) {
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

LambdaZr::LambdaZr() = default;

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

LambdaZrt::LambdaZrt() = default;

void LambdaZrt::randomIndependentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan) {
	CrossoverPartitionType ctype = Utils::randBool() ? CrossoverPartitionType::PERIOD_WISE : CrossoverPartitionType::RESOURCE_WISE;
	int qj = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
	int q2 = Utils::randRangeIncl(0, (ctype == CrossoverPartitionType::PERIOD_WISE ? heuristicMaxMakespan : z.getM()) - 1);
	independentOnePointCrossovers(mother, father, qj, q2, ctype);
}

void LambdaZrt::independentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int qj, int q2, CrossoverPartitionType ctype) {
	onePointCrossover(mother, father, qj);
	switch(ctype) {
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

void LambdaZrt::randomIndependentTwoPointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan) {
	CrossoverPartitionType ctype = Utils::randBool() ? CrossoverPartitionType::PERIOD_WISE : CrossoverPartitionType::RESOURCE_WISE;

	pair<int,int> qj;
	qj.first = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 2);
	qj.second = Utils::randRangeIncl(qj.first + 1, static_cast<int>(order.size()) - 1);

	pair<int,int> q2;
	int q2ub = (ctype == CrossoverPartitionType::PERIOD_WISE ? heuristicMaxMakespan : z.getM()) - 1;
	q2.first = Utils::randRangeIncl(0, q2ub);
	q2.second = Utils::randRangeIncl(min(q2.first+1, q2ub), q2ub);

	independentTwoPointCrossovers(mother, father, qj, q2, ctype);
}

void LambdaZrt::independentTwoPointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, pair<int,int> qj, pair<int,int> q2, CrossoverPartitionType ctype) {
	twoPointCrossover(mother, father, qj.first, qj.second);
	switch(ctype) {
		case CrossoverPartitionType::RESOURCE_WISE:
			z.foreachAssign([&](int r, int t) {
				return r <= q2.first || r > q2.second ? mother.z(r, t) : father.z(r, t);
			});
			break;
		case CrossoverPartitionType::PERIOD_WISE:
			z.foreachAssign([&](int r, int t) {
				return t <= q2.first || t > q2.second ? mother.z(r, t) : father.z(r, t);
			});
			break;
	}
}

void LambdaZrt::independentMutations(const Matrix<char>& adjMx, const vector<int> &zmax, int pmutate, bool keepTopOrder) {
	neighborhoodSwap(adjMx, pmutate, keepTopOrder);
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

LambdaTau::LambdaTau() = default;

void LambdaTau::inherit(const Lambda &parent, int destIx, int srcIx) {
    Lambda::inherit(parent, destIx, srcIx);
    tau[destIx] = static_cast<const LambdaTau &>(parent).tau[srcIx];
}

void LambdaTau::swap(int i1, int i2) {
    Lambda::swap(i1, i2);
    Utils::swap(tau, i1, i2);
}

//======================================================================================================================

RandomKey::RandomKey(int numJobs) : priorities(numJobs) {}
RandomKey::RandomKey(const std::vector<float>& _priorities): priorities(_priorities) {}
RandomKey::RandomKey() = default;

void RandomKey::mutate(int pmutate) {
	for (int i = 1; i < priorities.size(); i++) {
		if (Utils::randRangeIncl(1, 100) <= pmutate) {
			priorities[i] = Utils::randUnitFloat();
		}
	}
	
}

void RandomKey::randomOnePointCrossover(const RandomKey& mother, const RandomKey& father) {
	int q = Utils::randRangeIncl(0, static_cast<int>(priorities.size()) - 1);
	onePointCrossover(mother, father, q);
}

void RandomKey::onePointCrossover(const RandomKey& mother, const RandomKey& father, int q) {
	for(int i = 0; i < priorities.size(); i++)
		priorities[i] = i <= q ? mother.priorities[i] : father.priorities[i];
}

//======================================================================================================================

// TODO: Refactor random key codepath for reduced redundancy

RandomKeyZr::RandomKeyZr(int numJobs, int numRes) : RandomKey(numJobs), z(numRes) {}

RandomKeyZr::RandomKeyZr() = default;

void RandomKeyZr::randomIndependentOnePointCrossovers(const RandomKeyZr& mother, const RandomKeyZr& father) {
	int qj = Utils::randRangeIncl(0, static_cast<int>(priorities.size()) - 1);
	int qr = Utils::randRangeIncl(0, static_cast<int>(z.size()) - 1);
	independentOnePointCrossovers(mother, father, qj, qr);
}

void RandomKeyZr::independentOnePointCrossovers(const RandomKeyZr& mother, const RandomKeyZr& father, int qj, int qr) {
	onePointCrossover(mother, father, qj);
	for (int r = 0; r < z.size(); r++)
		z[r] = r <= qr ? mother.z[r] : father.z[r];
}

//======================================================================================================================

RandomKeyZrt::RandomKeyZrt(int numJobs, int numRes, int numPeriods) : RandomKey(numJobs), z(numRes, numPeriods) {}
RandomKeyZrt::RandomKeyZrt() = default;
RandomKeyZrt::RandomKeyZrt(const std::vector<float>& _order, const Matrix<int>& _z): RandomKey(_order), z(_z) {}

void RandomKeyZrt::randomIndependentOnePointCrossovers(const RandomKeyZrt& mother, const RandomKeyZrt& father, int heuristicMaxMakespan) {
	CrossoverPartitionType ctype = Utils::randBool() ? CrossoverPartitionType::PERIOD_WISE : CrossoverPartitionType::RESOURCE_WISE;
	int qj = Utils::randRangeIncl(0, static_cast<int>(priorities.size()) - 1);
	int q2 = Utils::randRangeIncl(0, (ctype == CrossoverPartitionType::PERIOD_WISE ? heuristicMaxMakespan : z.getM()) - 1);
	independentOnePointCrossovers(mother, father, qj, q2, ctype);
}

void RandomKeyZrt::independentOnePointCrossovers(const RandomKeyZrt& mother, const RandomKeyZrt& father, int qj, int q2, CrossoverPartitionType ctype) {
	onePointCrossover(mother, father, qj);
	switch (ctype) {
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

void RandomKeyZrt::independentMutations(const Matrix<char>& adjMx, const std::vector<int>& zmax, int pmutate) {
	mutate(pmutate);
	for (int r = 0; r < z.getM(); r++) {
		int zOffset = Utils::randBool() ? 1 : -1;
		for (int t = 0; t < z.getN(); t++) {
			if (Utils::randRangeIncl(1, 100) <= pmutate) {
				z(r, t) = boost::algorithm::clamp(z(r, t) + zOffset, 0, zmax[r]);
			}
		}
	}
}

//======================================================================================================================

PartitionList::PartitionList(int numJobs) : plist(numJobs) {}

void PartitionList::combine(const PartitionList &mother, const PartitionList &father, int partitionSize) {
	int numPartitions = static_cast<int>(ceil((float)plist.size() / (float)partitionSize));
	int q = Utils::randRangeIncl(0, numPartitions-2);
	for(int j=0; j<plist.size(); j++) {
		plist[j] = mother.plist[j] <= q ? mother.plist[j] : -1;
	}

	int pix = q+1;
	for(int i=0; i<plist.size()-(q+1)*partitionSize; i++) {
		int j = lowestPartitionJobFromOtherNotAlreadyChosen(father.plist);
		assert(j != -1);
		plist[j] = pix;
		if((i+1) % partitionSize == 0) {
			pix++;
		}
	}
}

bool PartitionList::isFeasible(const Matrix<char> &adjMx) const {
	for(int i=0; i<adjMx.getM(); i++) {
		for(int j=0; j<adjMx.getN(); j++) {
			if(adjMx(i,j) && plist[i] > plist[j])
				return false;
		}
	}
	return true;
}

int PartitionList::determineOtherJobForSwap(int j, MoveDir dir) const {
	int npix = dir == MoveDir::LEFT ? plist[j] - 1 : plist[j] + 1;
	int numInOtherPartition = static_cast<int>(count_if(plist.begin(), plist.end(), [&npix](int itspix) { return itspix == npix; }));
	int nthChosen = Utils::randRangeIncl(0, numInOtherPartition-1);
	return Utils::indexOfNthEqualTo(nthChosen, npix, plist);
}

void PartitionList::partitionSwap(const Matrix<char> &adjMx, int pmutate, int partitionSize) {
	const auto numPartitions = static_cast<int>(ceil((float)adjMx.getM() / (float)partitionSize));

	for(int j=0; j<adjMx.getM(); j++) {
		if(Utils::randRangeIncl(1, 100) <= pmutate) {
			int pix = plist[j];

			MoveDir dir = Utils::randBool() ? MoveDir::LEFT : MoveDir::RIGHT;
			if(pix == 0) dir = MoveDir::RIGHT;
			else if(pix == numPartitions -1) dir = MoveDir::LEFT;

			int otherJob = determineOtherJobForSwap(j, dir);
			int npix = dir == MoveDir::LEFT ? plist[j] - 1 : plist[j] + 1;

			Utils::swap(plist, j, otherJob);
			if(!isFeasible(adjMx)) {
				Utils::swap(plist, j, otherJob);
			}
		}
	}
}

int PartitionList::lowestPartitionJobFromOtherNotAlreadyChosen(const std::vector<int> &other) {
	int lowestPartition = numeric_limits<int>::max();
	int lowestJob = -1;
	for(int j=0; j<plist.size(); j++) {
		if(plist[j] == -1 && other[j] < lowestPartition) {
			lowestPartition = other[j];
			lowestJob = j;
		}
	}
	return lowestJob;
}



