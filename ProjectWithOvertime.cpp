//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include <string>
#include "ProjectWithOvertime.h"

ProjectWithOvertime::ProjectWithOvertime(string filename) :
        Project(filename),
        zmax(numRes),
        kappa(numRes),
        revenue(numPeriods) {
    eachRes([&](int r) {
		zmax[r] = capacities[r] / 2;
        kappa[r] = 0.5f;
    });

    computeRevenueFunction();
}

inline float ProjectWithOvertime::totalCosts(const Matrix<int> & resRem) const {
	float costs = 0.0f;
    eachResPeriodConst([&](int r, int t) { costs += Utils::max(0, -resRem(r,t)) * kappa[r]; });
	return costs;
}

float ProjectWithOvertime::totalCosts(const vector<int> &sts) const {
	float costs = 0.0f;
	int cdemand;
	eachResPeriodConst([&](int r, int t) {
		cdemand = 0;
		eachJobConst([&](int j) {
			if (sts[j] < t && t <= sts[j] + durations[j])
				cdemand += demands(j, r);
		});
		costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
	});
	return costs;
}

float ProjectWithOvertime::totalCostsForPartial(const vector<int> &sts) const {
    float costs = 0.0f;
    int cdemand;
    eachResPeriodConst([&](int r, int t) {
        cdemand = 0;
        eachJobConst([&](int j) {
            if (sts[j] != UNSCHEDULED && sts[j] < t && t <= sts[j] + durations[j])
                cdemand += demands(j, r);
        });
        costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
    });
    return costs;
}

float ProjectWithOvertime::calcProfit(const vector<int> &sts) const {
	return revenue[makespan(sts)] - totalCosts(sts);
}

float ProjectWithOvertime::calcProfit(int makespan, const Matrix<int>& resRem) const {
	return revenue[makespan] - totalCosts(resRem);
}

void ProjectWithOvertime::computeRevenueFunction() {
    int tkappa = computeTKappa();
    Matrix<int> resRem(numRes, numPeriods);

    eachResPeriod([&](int r, int t) { resRem(r,t) = capacities[r]; });
    vector<int> ess = earliestStartSchedule(resRem);

    float maxCosts = totalCosts(resRem);

    int minMs = Utils::max(makespan(ess), tkappa);

    auto sts = serialSGS(topOrder);
    int maxMs = makespan(sts);

    eachPeriod([&](int t) { revenue[t] = static_cast<float>(maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)); });
}

int ProjectWithOvertime::computeTKappa() {
    int tkappa = 0;
    eachRes([&](int r) {
		float tkappar = 0.0f;
        eachJob([&](int j) { tkappar += durations[j] * demands(j,r); });
		tkappar /= static_cast<float>(capacities[r] + zmax[r]);
		tkappar = ceil(tkappar);
		tkappa = Utils::max(tkappa, static_cast<int>(tkappar));
	});
    return tkappa;
}

vector<int> ProjectWithOvertime::earliestStartingTimesForPartial(const vector<int>& sts) const {
	vector<int> ests(numJobs);

	for(int j : topOrder) {
		if(sts[j] != -1) {
			ests[j] = sts[j];
		}
	}

	for (int j : topOrder) {
		if(sts[j] != -1) continue;
		ests[j] = 0;
		for (int i = 0; i<numJobs; i++) {
			if (adjMx(i, j))
				ests[j] = Utils::max(ests[j], ests[i] + durations[i]);
		}
	}

	return ests;
}

vector<int> ProjectWithOvertime::latestFinishingTimesForPartial(const vector<int>& sts) const {
	vector<int> lfts(numJobs);
	
	for (int i : revTopOrder) {
		if (sts[i] != -1) {
			lfts[i] = sts[i];
		}
	}

	for (int i : revTopOrder) {
		if (sts[i] != -1) continue;
		lfts[i] = T;
		eachJobConst([&](int j){
			if(adjMx(i, j))
				lfts[i] = Utils::min(lfts[i], lfts[j] - durations[j]);
		});
	}
	
	return lfts;
}

list<int> ProjectWithOvertime::decisionTimesForResDevProblem(const vector<int>& sts, const vector<int>& ests, const vector<int>& lfts, int j) const {
	list<int> decisionTimes = { ests[j], lfts[j] };

	for(int tau = ests[j]; tau <= lfts[j]; tau++) {
		eachJobConst([&](int i) {
			if (sts[i] + durations[i] == tau || tau + durations[j] == sts[i])
				decisionTimes.push_back(tau);
		});
	}

	return decisionTimes;
}

float ProjectWithOvertime::extensionCosts(const Matrix<int> &resRem, int j, int stj) const {
	float costs = 0.0f;
	eachResConst([&](int r) {
		for (int tau = stj + 1; tau <= stj + durations[j]; tau++) {
			costs += Utils::min(0, demands(j,r) - resRem(r,tau)) * kappa[r];
		}
	});	
	return costs;
}

int ProjectWithOvertime::selectBestStartingTime(vector<int>& sts, int j, const list<int>& decisionTimes, const Matrix<int> &resRem) const {
	int bestT;
	float bestCosts = numeric_limits<float>::max(), costs;
	for(auto dt : decisionTimes) {
		costs = extensionCosts(resRem, j, dt);
		if(costs < bestCosts) {
			bestCosts = costs;
			bestT = dt;
		}
	}
	return bestT;
}

SGSResult ProjectWithOvertime::serialSGSWithOvertime(const vector<int> &order) {
    Matrix<int> resRem(numRes, numPeriods);
    eachResPeriod([&](int r, int t) { resRem(r,t) = capacities[r]; });

    vector<int> sts(numJobs), fts(numJobs), ftsTmp(numJobs);
    for (int k=0; k<numJobs; k++) {
        int job = order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);

        int t;
        for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);

        pair<int, float> bestT = make_pair(t, numeric_limits<float>::lowest());

        for(;; t++) {
            if(!enoughCapacityForJobWithOvertime(job, t, resRem))
                continue;

            Matrix<int> resRemTmp(resRem);
            ftsTmp.swap(fts);
            complementPartialWithSSGS(order, k, ftsTmp, resRemTmp);

            float p = calcProfit(makespan(fts), resRemTmp);
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

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, Matrix<int> & resRem) const {
    for(int tau = t + 1; tau <= t + durations[job]; tau++) {
        for(int r=0; r<numRes; r++)
            if(demands(job,r) > resRem(r,tau) + zmax[r])
                return false;
    }
    return true;
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta) {
    Matrix<int> resRem(numRes, numPeriods);
    eachResPeriod([&](int r, int t) {resRem(r,t) = capacities[r]; });

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

SGSResult ProjectWithOvertime::serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau) {
    Matrix<int> resRem(numRes, numPeriods);
    eachResPeriod([&](int r, int t) { resRem(r,t) = capacities[r]; });

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

// FIXME: Implement me!
SGSResult ProjectWithOvertime::serialSGSWithDeadline(int deadline, const vector<int> order) {
    Matrix<int> resRem(numRes, numPeriods);
    eachResPeriod([&](int r, int t) { resRem(r,t) = capacities[r]; });
	vector<int> sts(numJobs, -1);

    for(int i=0; i<numJobs; i++) {
		vector<int> cests = earliestStartingTimesForPartial(sts);
		vector<int> clfts = latestFinishingTimesForPartial(sts);

		int job = order[i];
		list<int> decisionTimes = decisionTimesForResDevProblem(sts, cests, clfts, job);
		if(!decisionTimes.empty()) {
		}       

        int t = 0;

        sts[job] = t;
    }

    return make_pair(sts, resRem);
}

