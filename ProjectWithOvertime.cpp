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
        zmax[r] = ceil(capacities[r] / 2.0f);
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

    int minMs = max(ess[numJobs-1], tkappa);

    auto sts = serialSGS(topOrder, zeroOc);
    int maxMs = sts[numJobs-1];

    EACH_PERIOD(revenue[t] = (float) (maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

int ProjectWithOvertime::computeTKappa() const {
    int tkappa = 0;
    EACH_RES(
        int tkappar = 0;
        for(int j=0; j< numJobs -1; j++)
            tkappar += durations[j] * demands[j][r];

        tkappar /= capacities[r] + zmax[r];
        tkappar = (int)ceil(tkappar);
        tkappa = max(tkappa, tkappar);
    )
    return tkappa;
}

vector<int> ProjectWithOvertime::earliestStartSchedule(vector<vector<int>>& resRem) {
    vector<int> ess((unsigned long) numJobs);

    EACH_JOB(
        ess[j] = 0;
        EACH_JOBi(if(adjMx[i][j] && ess[i] + durations[i] > ess[j]) ess[j] = ess[i] + durations[i])
        EACH_RES(for(int tau=ess[j]+1; tau<ess[j]+durations[j]; tau++) resRem[r][tau] -= demands[j][r])
    )
    return ess;
}

float ProjectWithOvertime::totalCosts(vector<vector<int>> resRem) {
    float costs = 0.0f;
    EACH_RES(EACH_PERIOD(costs += min(0, resRem[r][t]) * kappa[r]))
    return costs;
}
