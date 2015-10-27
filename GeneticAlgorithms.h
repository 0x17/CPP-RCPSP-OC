//
// Created by André Schnabel on 25.10.15.
//

#ifndef CPP_RCPSP_OC_GENETICALGORITHMS_H
#define CPP_RCPSP_OC_GENETICALGORITHMS_H

#include <vector>
using namespace std;

template<class Individual>
class GeneticAlgorithm {
public:
	virtual ~GeneticAlgorithm() {}

	Individual solve(int numGens, int popSize, int pmutate) {
        vector<Individual> population(popSize*2);

        for(int i=0; i<popSize; i++) {
            population[i] = init(i);
        }

        for(int i=0; i<numGens; i++) {
            // Crossover 2 parents (0..popSize-1) into children (popSize..popSize*2-1)
            // Mutate children
            // Compute fitness for all
            // Sort descending fitness
        }

        return population[0];
    }

protected:
    virtual Individual init(int ix) = 0;
    virtual Individual crossover(Individual a, Individual b) = 0;
    virtual Individual mutate(Individual i) = 0;
    virtual float fitness(Individual i) = 0;
};

struct LambdaZr {
    vector<int> order, z;
};

struct LambdaZrt {
    vector<int> order;
    vector<vector<int>> z;
};

struct LambdaBeta {
    vector<int> order, beta;
};

struct LambdaTau {
    vector<int> order;
    vector<float> tau;
};

struct DeadlineLambda {
    int deadline;
    vector<int> order;
};

class FixedCapacityGA : GeneticAlgorithm<LambdaZr> {
private:
    virtual LambdaZr init(int ix) override;
    virtual LambdaZr crossover(LambdaZr a, LambdaZr b) override;
    virtual LambdaZr mutate(LambdaZr i) override;
    virtual float fitness(LambdaZr i) override;
};

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
