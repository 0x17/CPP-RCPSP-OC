//
// Created by André Schnabel on 06.02.16.
//

#pragma once

#include "../Matrix.h"
#include "../Utils.h"
#include "../ProjectWithOvertime.h"

class Lambda {
public:
    vector<int> order;

    Lambda(int numJobs);
    Lambda(const vector<int> &_order);
    Lambda();
    virtual ~Lambda() {}

    virtual void neighborhoodSwap(const Matrix<char> &adjMx, int pmutate);

    virtual void randomOnePointCrossover(const Lambda &mother, const Lambda& father);
    virtual void onePointCrossover(const Lambda &mother, const Lambda& father, int q);

	template<class T>
	static void onePointCrossoverLists(int q, vector<T> &daughter, const vector<T> &mother, const vector<T> &father);

	virtual void randomTwoPointCrossover(const Lambda &mother, const Lambda &father);
    virtual void twoPointCrossover(const Lambda &mother, const Lambda &father, int q1, int q2);

    virtual void inherit(const Lambda &parent, int destIx, int srcIx);
    virtual void swap(int i1, int i2);
};

template <class T>
void Lambda::onePointCrossoverLists(int q, vector<T> &daughter, const vector<T> &mother, const vector<T> &father) {
	for(int i = 0; i <= q; i++) {
		daughter[i] = mother[i];
	}

	for(int i = 0, ctr = q + 1; i < daughter.size(); i++) {
		if(!Utils::rangeInclContains(daughter, 0, q, father[i])) {
			daughter[ctr] = father[i];
			ctr++;
		}
	}
}

class DeadlineLambda : public Lambda {
public:
    int deadlineOffset;

    DeadlineLambda(int numJobs);
    DeadlineLambda();
    virtual ~DeadlineLambda() {}

    virtual void randomOnePointCrossover(const Lambda &mother, const Lambda &father) override;
};

class LambdaZr : public Lambda {
public:
    vector<int> z;

    LambdaZr(int numJobs, int numRes);
    LambdaZr();
	LambdaZr(const vector<int> &_order, const vector<int> &_z) : Lambda(_order), z(_z) {}
    virtual ~LambdaZr() {}

	void randomIndependentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father);
	void independentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father, int qj, int qr);
};

class LambdaZrt : public Lambda  {
public:
    Matrix<int> z;

    LambdaZrt(int numJobs, int numRes, int numPeriods);
    LambdaZrt();
	LambdaZrt(const vector<int> &_order, const Matrix<int> &_z) : Lambda(_order), z(_z) {}
    virtual ~LambdaZrt() {}

	enum class CrossoverPartitionType {
		RESOURCE_WISE,
		PERIOD_WISE
	};

	void randomIndependentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan);
	void independentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int qj, int q2, CrossoverPartitionType ctype);

	void independentMutations(const Matrix<char>& adjMx, const vector<int> &zmax, int pmutate);
};

class LambdaBeta : public Lambda {
public:
    vector<int> beta;

    LambdaBeta(int numJobs);
    LambdaBeta() {}
	LambdaBeta(const vector<int> &_order, const vector<int> &_beta) : Lambda(_order), beta(_beta) {}
    virtual ~LambdaBeta() {}

    virtual void inherit(const Lambda &parent, int destIx, int srcIx) override;
    virtual void swap(int i1, int i2) override;

	void separateOnePointCrossover(const LambdaBeta& mother, const LambdaBeta& father);

	static ProjectWithOvertime::BorderSchedulingOptions options;
	static void setOptions(ProjectWithOvertime::BorderSchedulingOptions _options);
};

class LambdaTau : public Lambda {
public:
    vector<float> tau;

    LambdaTau(int numJobs);
    LambdaTau();
    virtual ~LambdaTau() {}

    virtual void inherit(const Lambda &parent, int destIx, int srcIx) override;
    virtual void swap(int i1, int i2) override;
};

