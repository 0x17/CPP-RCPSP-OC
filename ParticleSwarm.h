#pragma once

#include "Project.h"

class ProjectWithOvertime;

class ParticleSwarm
{
	ProjectWithOvertime &p;
public:
	ParticleSwarm(ProjectWithOvertime &p);
	~ParticleSwarm();

	SGSResult solve();
};

