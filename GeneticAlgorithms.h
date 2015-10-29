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
    virtual void crossover(Individual &mother, Individual &father, Individual &daughter) = 0;
    virtual void mutate(Individual &i) = 0;
    virtual float fitness(Individual &i) = 0;
};

struct LambdaZrt { vector<int> order; vector<vector<int>> z; };
struct LambdaBeta { vector<int> order, beta; };
struct LambdaTau { vector<int> order; vector<float> tau; };
struct DeadlineLambda { int deadline; vector<int> order; };
struct LambdaZr { vector<int> order, z; };

class TimeVaryingCapacityGA : public GeneticAlgorithm<LambdaZrt> {
	LambdaZrt init(int ix) override { LambdaZrt e; return e; }
	void crossover(LambdaZrt& mother, LambdaZrt& father, LambdaZrt& daughter) override {}
	void mutate(LambdaZrt& i) override {}
	float fitness(LambdaZrt& i) override { return 0.0f;  }
};

class FixedCapacityGA : public GeneticAlgorithm<LambdaZr> {
	virtual LambdaZr init(int ix) override { LambdaZr e; return e; }
    virtual void crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) override {}
    virtual void mutate(LambdaZr &i) override {}
	virtual float fitness(LambdaZr &i) override { return 0.0f;  }
};

#endif //CPP_RCPSP_OC_GENETICALGORITHMS_H
