//
// Created by André Schnabel on 06.02.16.
//

#ifndef CPP_RCPSP_OC_REPRESENTATIONS_H
#define CPP_RCPSP_OC_REPRESENTATIONS_H

#include "../Matrix.h"
#include "../Project.h"
#include "../Utils.h"

class Lambda {
public:
    vector<int> order;

    Lambda(Project &p) : order(p.numJobs) {}
    Lambda() {}

    virtual ~Lambda() {}

    virtual void neighborhoodSwap(Matrix<char> &adjMx, int pmutate) {
        for(int i=1; i<order.size(); i++) {
            if (Utils::randRangeIncl(1, 100) <= pmutate && !adjMx(order[i - 1], order[i])) {
                int tmp = order[i-1];
                order[i-1] = order[i];
                order[i] = tmp;
            }
        }
    }

    virtual void randomOnePointCrossover(Lambda &mother, Lambda& father) {
        int q = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 1);
        onePointCrossover(mother, father, q);
    }

    virtual void onePointCrossover(Lambda &mother, Lambda& father, int q) {
        for(int i = 0; i <= q; i++) { inherit(mother, i, i); }

        for(int i = 0, ctr = q + 1; i<order.size(); i++) {
            if(!Utils::rangeInclContains(order, 0, q, father.order[i])) {
                inherit(father, ctr, i);
                ctr++;
            }
        }
    }

    virtual void inherit(Lambda &parent, int destIx, int srcIx) {
        order[destIx] = parent.order[srcIx];
    }

    virtual void randomTwoPointCrossover(Lambda &mother, Lambda &father) {
        int q1 = Utils::randRangeIncl(0, static_cast<int>(order.size()) - 2);
        int q2 = Utils::randRangeIncl(q1 + 1, static_cast<int>(order.size()) - 1);
        twoPointCrossover(mother, father, q1, q2);
    }

    virtual void twoPointCrossover(Lambda &mother, Lambda &father, int q1, int q2) {
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
};

class DeadlineLambda : public Lambda {
public:
    int deadline;

    DeadlineLambda(Project &p) : Lambda(p), deadline(0) {}
    DeadlineLambda() {}
};

class LambdaZr : public Lambda {
public:
    vector<int> z;

    LambdaZr(Project &p) : Lambda(p), z(p.numRes) {}
    LambdaZr() {}

};
class LambdaZrt : public Lambda  {
public:
    Matrix<int> z;

    LambdaZrt(Project &p) : Lambda(p), z(p.numRes, p.numPeriods) {}
    LambdaZrt() {}

};

class LambdaBeta : public Lambda {
public:
    vector<int> beta;

    LambdaBeta(Project &p) : Lambda(p), beta(p.numJobs) {}
    LambdaBeta() {}
    virtual ~LambdaBeta() {}


    virtual void inherit(Lambda &parent, int destIx, int srcIx) override {
        Lambda::inherit(parent, destIx, srcIx);
        beta[destIx] = ((LambdaBeta &)parent).beta[srcIx];
    }
};

class LambdaTau : public Lambda {
public:
    vector<float> tau;

    LambdaTau(Project &p) : Lambda(p), tau(p.numJobs) {}
    LambdaTau() {}
    virtual ~LambdaTau() {}

    virtual void inherit(Lambda &parent, int destIx, int srcIx) override {
        Lambda::inherit(parent, destIx, srcIx);
        tau[destIx] = ((LambdaTau &)parent).tau[srcIx];
    }
};



#endif //CPP_RCPSP_OC_REPRESENTATIONS_H
