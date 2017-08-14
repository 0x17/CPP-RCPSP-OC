#pragma once

#include "../Project.h"

class ProjectWithOvertime;

class ParticleSwarm
{	
public:
	ParticleSwarm(ProjectWithOvertime &p);
	~ParticleSwarm();

	SGSResult solve();

	static double distance(const std::vector<int>& x1, const std::vector<int>& x2);
	static void sortDescendingFitness(std::list<std::pair<std::vector<int>, float>>& popWithFitness);
	static void pathRelinking(const std::vector<int>& x1, const std::vector<int>& x2, std::list<std::pair<std::vector<int>, float>> &popWithFitness);
	
	void feasibleMaintain(int job, std::vector<int>& x) const;
	int successorOfJobWithMinPosition(int job, const std::vector<int>& x) const;

private:
	ProjectWithOvertime &p;

	std::vector<std::vector<int>> generateSwarm(int nsw, int iter, std::list<std::pair<std::vector<int>, float>>& popWithFitness) const;
};

