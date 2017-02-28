#pragma once

#include "Project.h"

class ProjectWithOvertime;

class ParticleSwarm
{	
public:
	ParticleSwarm(ProjectWithOvertime &p);
	~ParticleSwarm();

	SGSResult solve();

	static double distance(const vector<int>& x1, const vector<int>& x2);
	static void sortDescendingFitness(list<pair<vector<int>, float>>& popWithFitness);
	static void pathRelinking(const vector<int>& x1, const vector<int>& x2, list<pair<vector<int>, float>> &popWithFitness);
	
	void feasibleMaintain(int job, vector<int>& x) const;
	int successorOfJobWithMinPosition(int job, const vector<int>& x) const;

private:
	ProjectWithOvertime &p;

	vector<vector<int>> generateSwarm(int nsw, int iter, list<pair<vector<int>, float>>& popWithFitness) const;
};

