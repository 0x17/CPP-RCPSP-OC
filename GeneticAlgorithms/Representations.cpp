//
// Created by André Schnabel on 06.02.16.
//

#include <cmath>
#include "Representations.h"

Lambda::Lambda(int numJobs) : order(numJobs) {}

Lambda::Lambda(vector<int> _order) : order(_order) {}

Lambda::Lambda() {}

void Lambda::neighborhoodSwap(Matrix<char> &adjMx, int pmutate) {
    for(int i=1; i<order.size(); i++) {
        if(Utils::randRangeIncl(1, 100) <= pmutate && !adjMx(order[i - 1], order[i])) {
            swap(i-1, i);
        }
    }
}

void Lambda::randomOnePointCrossover(Lambda &mother, Lambda& father) {
    int q = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
    onePointCrossover(mother, father, q);
}

void Lambda::onePointCrossover(Lambda &mother, Lambda& father, int q) {
    for(int i = 0; i <= q; i++) { inherit(mother, i, i); }

    for(int i = 0, ctr = q + 1; i<order.size(); i++) {
        if(!Utils::rangeInclContains(order, 0, q, father.order[i])) {
            inherit(father, ctr, i);
            ctr++;
        }
    }
}

void Lambda::inherit(Lambda &parent, int destIx, int srcIx) {
    order[destIx] = parent.order[srcIx];
}

void Lambda::randomTwoPointCrossover(Lambda &mother, Lambda &father) {
    int q1 = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 2);
    int q2 = Utils::randRangeIncl(q1 + 1, static_cast<int>(order.size()) - 1);
    twoPointCrossover(mother, father, q1, q2);
}

void Lambda::twoPointCrossover(Lambda &mother, Lambda &father, int q1, int q2) {
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

DeadlineLambda::DeadlineLambda(int numJobs) : Lambda(numJobs), deadline(0) {}

DeadlineLambda::DeadlineLambda(): deadline(0) {}

void DeadlineLambda::randomOnePointCrossover(Lambda &mother, Lambda &father) {
    Lambda::randomOnePointCrossover(mother, father);
    auto &m = static_cast<DeadlineLambda&>(mother);
    auto &f = static_cast<DeadlineLambda&>(father);
    deadline = static_cast<int>(std::round(static_cast<float>(m.deadline - f.deadline) / 2.0f)) + f.deadline;
}

//======================================================================================================================

LambdaZr::LambdaZr(int numJobs, int numRes) : Lambda(numJobs), z(numRes) {}

LambdaZr::LambdaZr() {}

//======================================================================================================================

LambdaZrt::LambdaZrt(int numJobs, int numRes, int numPeriods) : Lambda(numJobs), z(numRes, numPeriods) {}

LambdaZrt::LambdaZrt() {}

//======================================================================================================================

LambdaBeta::LambdaBeta(int numJobs) : Lambda(numJobs), beta(numJobs) {}

void LambdaBeta::inherit(Lambda &parent, int destIx, int srcIx) {
    Lambda::inherit(parent, destIx, srcIx);
    beta[destIx] = static_cast<LambdaBeta &>(parent).beta[srcIx];
}

void LambdaBeta::swap(int i1, int i2) {
    Lambda::swap(i1, i2);
    Utils::swap(beta, i1, i2);
}

//======================================================================================================================

LambdaTau::LambdaTau(int numJobs) : Lambda(numJobs), tau(numJobs) {}

LambdaTau::LambdaTau() {}

void LambdaTau::inherit(Lambda &parent, int destIx, int srcIx) {
    Lambda::inherit(parent, destIx, srcIx);
    tau[destIx] = static_cast<LambdaTau &>(parent).tau[srcIx];
}

void LambdaTau::swap(int i1, int i2) {
    Lambda::swap(i1, i2);
    Utils::swap(tau, i1, i2);
}
