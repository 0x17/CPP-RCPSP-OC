//
// Created by André Schnabel on 06.02.16.
//

#pragma once

#include "../Matrix.h"
#include "../Utils.h"
#include "../ProjectWithOvertime.h"

class Lambda {
public:
    std::vector<int> order;

	explicit Lambda(int numJobs);
	explicit Lambda(const std::vector<int> &_order);
    Lambda();
    virtual ~Lambda() = default;

	virtual void neighborhoodSwap(const Matrix<char> &adjMx, int pmutate, bool keepTopOrder);

    virtual void randomOnePointCrossover(const Lambda &mother, const Lambda& father);
    virtual void onePointCrossover(const Lambda &mother, const Lambda& father, int q);

	template<class T>
	static void onePointCrossoverLists(int q, std::vector<T> &daughter, const std::vector<T> &mother, const std::vector<T> &father);

	virtual void randomTwoPointCrossover(const Lambda &mother, const Lambda &father);
    virtual void twoPointCrossover(const Lambda &mother, const Lambda &father, int q1, int q2);

    virtual void inherit(const Lambda &parent, int destIx, int srcIx);
    virtual void swap(int i1, int i2);
};

template <class T>
void Lambda::onePointCrossoverLists(int q, std::vector<T> &daughter, const std::vector<T> &mother, const std::vector<T> &father) {
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

	explicit DeadlineLambda(int numJobs);
    DeadlineLambda();
    ~DeadlineLambda() final = default;

	void randomOnePointCrossover(const Lambda &mother, const Lambda &father) override;
};

class LambdaZr : public Lambda {
public:
    std::vector<int> z;

    LambdaZr(int numJobs, int numRes);
    LambdaZr();
	LambdaZr(const std::vector<int> &_order, const std::vector<int> &_z) : Lambda(_order), z(_z) {}
    ~LambdaZr() final = default;

	void randomIndependentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father);
	void independentOnePointCrossovers(const LambdaZr& mother, const LambdaZr& father, int qj, int qr);
};

class LambdaZrt : public Lambda  {
public:
    Matrix<int> z;

    LambdaZrt(int numJobs, int numRes, int numPeriods);
    LambdaZrt();
	LambdaZrt(const std::vector<int> &_order, const Matrix<int> &_z) : Lambda(_order), z(_z) {}
    ~LambdaZrt() final = default;

	enum class CrossoverPartitionType {
		RESOURCE_WISE,
		PERIOD_WISE
	};

	void randomIndependentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan);
	void independentOnePointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int qj, int q2, CrossoverPartitionType ctype);

	void randomIndependentTwoPointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, int heuristicMaxMakespan);
	void independentTwoPointCrossovers(const LambdaZrt& mother, const LambdaZrt& father, std::pair<int,int> qj, std::pair<int,int> q2, CrossoverPartitionType ctype);

	void independentMutations(const Matrix<char>& adjMx, const std::vector<int> &zmax, int pmutate, bool keepTopOrder = true);
};

class LambdaBeta : public Lambda {
public:
    std::vector<int> beta;

	explicit LambdaBeta(int numJobs);
    LambdaBeta() = default;
	LambdaBeta(const std::vector<int> &_order, const std::vector<int> &_beta) : Lambda(_order), beta(_beta) {}
    ~LambdaBeta() final = default;

	void inherit(const Lambda &parent, int destIx, int srcIx) override;
    void swap(int i1, int i2) override;

	void separateOnePointCrossover(const LambdaBeta& mother, const LambdaBeta& father);
};

class LambdaTau : public Lambda {
public:
    std::vector<float> tau;

    explicit LambdaTau(int numJobs);
	explicit LambdaTau();
    ~LambdaTau() final = default;

    void inherit(const Lambda &parent, int destIx, int srcIx) override;
    void swap(int i1, int i2) override;
};

// TODO: Refactor random key codepath for reduced redundancy

class RandomKey {
public:
	std::vector<float> priorities;

	explicit RandomKey(int numJobs);
	explicit RandomKey(const std::vector<float>& _priorities);
	RandomKey();
	virtual ~RandomKey() = default;

	void mutate(int pmutate);

	virtual void randomOnePointCrossover(const RandomKey& mother, const RandomKey& father);
	virtual void onePointCrossover(const RandomKey& mother, const RandomKey& father, int q);
};

class RandomKeyZr : public RandomKey {
public:
	std::vector<int> z;

	RandomKeyZr(int numJobs, int numRes);
	RandomKeyZr();
	RandomKeyZr(const std::vector<float> &_order, const std::vector<int> &_z) : RandomKey(_order), z(_z) {}
	~RandomKeyZr() final = default;

	void randomIndependentOnePointCrossovers(const RandomKeyZr& mother, const RandomKeyZr& father);
	void independentOnePointCrossovers(const RandomKeyZr& mother, const RandomKeyZr& father, int qj, int qr);
};


class RandomKeyZrt : public RandomKey {
public:
	Matrix<int> z;

	RandomKeyZrt(int numJobs, int numRes, int numPeriods);
	RandomKeyZrt();
	RandomKeyZrt(const std::vector<float>& _order, const Matrix<int>& _z);
	~RandomKeyZrt() final = default;

	enum class CrossoverPartitionType {
		RESOURCE_WISE,
		PERIOD_WISE
	};

	void randomIndependentOnePointCrossovers(const RandomKeyZrt& mother, const RandomKeyZrt& father, int heuristicMaxMakespan);
	void independentOnePointCrossovers(const RandomKeyZrt& mother, const RandomKeyZrt& father, int qj, int q2, CrossoverPartitionType ctype);
	void independentMutations(const Matrix<char>& adjMx, const std::vector<int>& zmax, int pmutate);
};