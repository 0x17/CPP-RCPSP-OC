//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include "ProjectWithOvertime.h"

ProjectWithOvertime::ProjectWithOvertime(string filename) :
        Project(filename),
        zmax(numRes),
        kappa(numRes),
        revenue(numPeriods) {
	EACH_RES(
		zmax[r] = capacities[r] / 2;
        kappa[r] = 0.5f;
    )

    computeRevenueFunction();
}

inline float ProjectWithOvertime::totalCosts(const vector<vector<int>> & resRem) {
	float costs = 0.0f;
	EACH_RES(EACH_PERIOD(costs += Utils::max(0, -resRem[r][t]) * kappa[r]))
	return costs;
}

float ProjectWithOvertime::calcProfit(int makespan, const vector<vector<int>>& resRem) {
	return revenue[makespan] - totalCosts(resRem);
}

void ProjectWithOvertime::computeRevenueFunction() {
    int tkappa = computeTKappa();
    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);

    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r]))
    vector<int> ess = earliestStartSchedule(resRem);

    float maxCosts = totalCosts(resRem);

    int minMs = Utils::max(ess[numJobs-1], tkappa);

    auto sts = serialSGS(topOrder);
    int maxMs = sts[numJobs-1];

    EACH_PERIOD(revenue[t] = static_cast<float>(maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

int ProjectWithOvertime::computeTKappa() const {
    int tkappa = 0;
	EACH_RES(
		float tkappar = 0.0f;
		EACH_JOB(tkappar += durations[j] * demands[j][r])
		tkappar /= static_cast<float>(capacities[r] + zmax[r]);
		tkappar = ceil(tkappar);
		tkappa = Utils::max(tkappa, static_cast<int>(tkappar));
	)
    return tkappa;
}

vector<int> ProjectWithOvertime::earliestStartSchedule(vector<vector<int>>& resRem) {
    vector<int> ess(numJobs);
    for(int j : topOrder) {
        ess[j] = 0;
        EACH_JOBi(if (adjMx[i][j] && ess[i] + durations[i] > ess[j]) ess[j] = ess[i] + durations[i])
        EACH_RES(for (int tau = ess[j] + 1; tau <= ess[j] + durations[j]; tau++) resRem[r][tau] -= demands[j][r])
    }
    return ess;
}

SGSResult ProjectWithOvertime::serialSGSWithOvertime(vector<int> order) {
    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);
    vector<vector<int>> resRemTmp = Utils::initMatrix<int>(numRes, numPeriods);
    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r]))

    vector<int> sts(numJobs), fts(numJobs), ftsTmp(numJobs);
    for (int k=0; k<numJobs; k++) {
        int job = order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);

        int t;
        for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);

        pair<int, float> bestT = make_pair(t, numeric_limits<float>::min());

        for(;; t++) {
            if(!enoughCapacityForJobWithOvertime(job, t, resRem))
                continue;

            resRemTmp.swap(resRem);
            ftsTmp.swap(fts);
            complementPartialWithSSGS(order, k, ftsTmp, resRemTmp);

            float p = calcProfit(fts[numJobs-1], resRemTmp);
            if(p > bestT.second) {
                bestT.first = t;
                bestT.second = p;
            }

            if(enoughCapacityForJob(job, t, resRem))
                break;
        }

        scheduleJobAt(job, bestT.first, sts, fts, resRem);
    }

    return make_pair(sts, resRem);
}

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, vector<vector<int>> & resRem) const {
    for(int tau = t + 1; tau <= t + durations[job]; tau++) {
        EACH_RES(if(demands[job][r] > resRem[r][tau] + zmax[r]) return false)
    }
    return true;
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowBorders(vector<int> order, vector<int> beta) {
    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);
    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r]))

    vector<int> sts(numJobs), fts(numJobs);
    for (int k=0; k<numJobs; k++) {
        int job = order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);
        int t;
        if(beta[k] == 1) {
            for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);
        } else {
            for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
        }

        scheduleJobAt(job, t, sts, fts, resRem);
    }

    return make_pair(sts, resRem);
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowArbitrary(vector<int> order, vector<float> tau) {
    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);
    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r]))

    vector<int> sts(numJobs), fts(numJobs);
    for (int k=0; k<numJobs; k++) {
        int job = order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);
        int t;
        for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);
        int tmin = t;
        for (; !enoughCapacityForJob(job, t, resRem); t++);
        int tmax = t;
        for (t = tmax - static_cast<int>(round(static_cast<float>(tmax-tmin) * tau[k])); !enoughCapacityForJobWithOvertime(job, t, resRem); t++);
        scheduleJobAt(job, t, sts, fts, resRem);
    }

    return make_pair(sts, resRem);
}
