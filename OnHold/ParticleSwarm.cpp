// TODO: Implement full method! Follow original paper of pseudo PSO!

#include <cmath>

#include "ParticleSwarm.h"
#include "../GeneticAlgorithms/Sampling.h"
#include "../ProjectWithOvertime.h"

using namespace std;

ParticleSwarm::ParticleSwarm(ProjectWithOvertime& _p) : p(_p) {
}

ParticleSwarm::~ParticleSwarm()
{
}

SGSResult ParticleSwarm::solve() {
	const int MAX_ITERATIONS = 10;

	const int npop = 100;
	const int nsw = 20;

	float c1, c2, c3;
	c1 = c2 = c3 = 0.2f;

	auto popWithFitness = Utils::constructList<pair<std::vector<int>,float>>(npop, [&](int j) {
		auto order = Sampling::sample(true, p);
		SGSResult result = p.serialSGS(order, p.zzero);
		int makespan = p.makespan(result);
		auto res = p.forwardBackwardIterations(order, result, makespan);
		return make_pair(res.sts, p.calcProfit(res));
	});

	for(int iter = 1; iter <= MAX_ITERATIONS; iter++) {
		auto swarm = generateSwarm(nsw, iter, popWithFitness);
		popWithFitness.resize(1);
		//popWithFitness.clear();

		while (popWithFitness.size() < npop) {

			for (int i = 0; i < std::floor(c1 * nsw); i++) {
				//pathRelinking(popWithFitness[i], gbest);
			}

			if (iter > 1) {
				for (int i = 0; i < std::floor(c2 * nsw); i++) {
					//pathRelinking(popWithFitness[i], pbest[i]);
				}
			}

			for (int i = 0; i < c3 * nsw; i++) {
				//pathRelinking(popWithFitness[i], random[i]);
			}

		}
	}

	SGSResult res;
	return res;
}

std::vector<std::vector<int>> ParticleSwarm::generateSwarm(int nsw, int iter, std::list<pair<std::vector<int>, float>>& popWithFitness) const {
	std::map<std::vector<int>, double> distp;

	auto mindist = [](int i) {
		return i < 3 ? 1 : (i < 4 ? 2 : (i < 7 ? 3 : 4));
	};

	auto removeSimiliarFromPopulation = [&](const std::vector<int> &al) {
		for (auto it = popWithFitness.begin(); it != popWithFitness.end();) {
			auto pair = *it;
			double dp;
			if (distp.count(pair.first) == 0) {
				dp = distance(pair.first, al);
			}
			else {
				dp = std::min(distp[pair.first], distance(pair.first, al));
			}
			distp[pair.first] = dp;
			double d = mindist(iter);
			if (dp < d) {
				it = popWithFitness.erase(it);
			}
			else {
				++it;
			}
		}
	};
		
	sortDescendingFitness(popWithFitness);

	return Utils::constructVector<std::vector<int>>(nsw, [&](int j) {
		std::vector<int> al = popWithFitness.front().first;
		popWithFitness.pop_front();
		removeSimiliarFromPopulation(al);
		return al;
	});
}

double ParticleSwarm::distance(const std::vector<int>& x1, const std::vector<int>& x2) {
	int dsum = 0;
	for(int j=0; j<x1.size(); j++) {
		dsum += abs(Utils::indexOfFirstEqualTo(j, x1) - Utils::indexOfFirstEqualTo(j, x2));
	}
	return (double)dsum / (double)x1.size();
}

void ParticleSwarm::sortDescendingFitness(std::list<pair<std::vector<int>, float>>& popWithFitness) {
	popWithFitness.sort([](auto& left, auto& right) {
		return left.second < right.second;
	});
}

void ParticleSwarm::pathRelinking(const std::vector<int>& x1, const std::vector<int>& x2, std::list<pair<std::vector<int>, float>> &popWithFitness) {
	std::list<pair<std::vector<int>, float>> newSolutions;
	int r2 = static_cast<int>(Utils::randRangeIncl(1, x1.size() - 1));
	int r1 = static_cast<int>(Utils::randRangeIncl(0, r2 - 1));
	for(int i = r1; i <= r2; i++) {
		if(x1[i] != x2[i]) {
			//Utils::swap(x1, i, Utils::indexOfFirstEqualTo(x2[i], x2));
		}
	}

	// ...
}

void ParticleSwarm::feasibleMaintain(int job, std::vector<int>& x) const {
	int b = successorOfJobWithMinPosition(job, x);
	if(Utils::indexOfFirstEqualTo(job, x) > Utils::indexOfFirstEqualTo(b, x)) {
		Utils::swap(x, job, b);
		feasibleMaintain(b, x);
	}	
}

int ParticleSwarm::successorOfJobWithMinPosition(int job, const std::vector<int>& x) const {
	int posb = 0, b;
	p.eachJobConst([&](int succ) {
		if(p.adjMx(job, succ)) {
			int pos = Utils::indexOfFirstEqualTo(succ, x);
			if(pos <= posb) {
				posb = pos;
				b = x[pos];
			}
		}
	});
	return b;
}
