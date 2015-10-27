//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include "ProjectWithOvertime.h"

ProjectWithOvertime::ProjectWithOvertime(string filename) :
        Project(filename),
        zmax(numRes),
        kappa(numRes),
        revenue(numPeriods),
        zeroOc(numRes) {
	EACH_RES(
		zmax[r] = capacities[r] / 2;
        kappa[r] = 0.5f;
    )

    computeRevenueFunction();
}

void ProjectWithOvertime::computeRevenueFunction() {
    int tkappa = computeTKappa();
    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);

    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r]))
    vector<int> ess = earliestStartSchedule(resRem);

    float maxCosts = totalCosts(resRem);

    int minMs = Utils::max(ess[numJobs-1], tkappa);

    auto sts = serialSGS(topOrder, zeroOc);
    int maxMs = sts[numJobs-1];

    EACH_PERIOD(revenue[t] = static_cast<float>(maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

int ProjectWithOvertime::computeTKappa() const {
    int tkappa = 0;
	EACH_RES(
		float tkappar = 0.0f;
		EACH_JOB(tkappar += durations[j] * demands[j][r])
		tkappar /= static_cast<float>(capacities[r] + zmax[r]);
		tkappar = static_cast<int>(ceil(tkappar));
		tkappa = Utils::max(tkappa, tkappar);
	)
    return tkappa;
}

vector<int> ProjectWithOvertime::earliestStartSchedule(vector<vector<int>>& resRem) {
    vector<int> ess(numJobs);
    for(int k=0; k<numJobs; k++) {
        int j = topOrder[k];
        ess[j] = 0;
        EACH_JOBi(if (adjMx[i][j] && ess[i] + durations[i] > ess[j]) ess[j] = ess[i] + durations[i])
        EACH_RES(for (int tau = ess[j] + 1; tau <= ess[j] + durations[j]; tau++) resRem[r][tau] -= demands[j][r])
    }
    return ess;
}

float ProjectWithOvertime::totalCosts(vector<vector<int>> resRem) {
    float costs = 0.0f;
    EACH_RES(EACH_PERIOD(costs += Utils::max(0, -resRem[r][t]) * kappa[r]))
    return costs;
}
