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

    Lambda(int numJobs);
    Lambda(vector<int> _order);
    Lambda();
    virtual ~Lambda() {}

    virtual void neighborhoodSwap(Matrix<char> &adjMx, int pmutate);
    virtual void randomOnePointCrossover(Lambda &mother, Lambda& father);
    virtual void onePointCrossover(Lambda &mother, Lambda& father, int q);
    virtual void inherit(Lambda &parent, int destIx, int srcIx);
    virtual void swap(int i1, int i2);
    virtual void randomTwoPointCrossover(Lambda &mother, Lambda &father);
    virtual void twoPointCrossover(Lambda &mother, Lambda &father, int q1, int q2);
};

class DeadlineLambda : public Lambda {
public:
    int deadline;

    DeadlineLambda(int numJobs);
    DeadlineLambda();
    virtual ~DeadlineLambda() {}

    virtual void randomOnePointCrossover(Lambda &mother, Lambda &father) override;
};

class LambdaZr : public Lambda {
public:
    vector<int> z;

    LambdaZr(int numJobs, int numRes);
    LambdaZr();
    virtual ~LambdaZr() {}
};
class LambdaZrt : public Lambda  {
public:
    Matrix<int> z;

    LambdaZrt(int numJobs, int numRes, int numPeriods);
    LambdaZrt();
    virtual ~LambdaZrt() {}
};

class LambdaBeta : public Lambda {
public:
    vector<int> beta;

    LambdaBeta(int numJobs);
    LambdaBeta() {}
    virtual ~LambdaBeta() {}

    virtual void inherit(Lambda &parent, int destIx, int srcIx) override;
    virtual void swap(int i1, int i2) override;
};

class LambdaTau : public Lambda {
public:
    vector<float> tau;

    LambdaTau(int numJobs);
    LambdaTau();
    virtual ~LambdaTau() {}

    virtual void inherit(Lambda &parent, int destIx, int srcIx) override;
    virtual void swap(int i1, int i2) override;
};



#endif //CPP_RCPSP_OC_REPRESENTATIONS_H
