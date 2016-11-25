#include "ParticleSwarm.h"
#include "GeneticAlgorithms/Sampling.h"
#include "ProjectWithOvertime.h"

ParticleSwarm::ParticleSwarm(ProjectWithOvertime& _p) : p(_p) {
}

ParticleSwarm::~ParticleSwarm()
{
}


SGSResult ParticleSwarm::solve() {
	const int npop = 100;
	const int nsw = 20;

	auto population = Utils::constructList<vector<int>>(npop, [&](int j) {
		auto order = Sampling::sample(true, p);
		SGSResult result = p.serialSGS(order, p.zzero);
		int makespan = p.makespan(result);
		return p.forwardBackwardIterations(order, result, makespan).sts;
	});

	auto swarm = Utils::constructVector<vector<int>>(nsw, [&](int j) {
		vector<int> al;
		return al;
	});
}
