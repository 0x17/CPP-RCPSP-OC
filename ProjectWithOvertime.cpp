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
    EACH_RES_PERIOD(costs += Utils::max(0, -resRem(r,t)) * kappa[r])
	return costs;
}

float ProjectWithOvertime::totalCosts(const vector<int> &sts) const {
	float costs = 0.0f;
	int cdemand;
	EACH_RES_PERIOD(
		cdemand = 0;
		EACH_JOB(if (sts[j] < t && t <= sts[j] + durations[j]) cdemand += demands(j, r))
		costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
    )
	return costs;
}

float ProjectWithOvertime::totalCostsForPartial(const vector<int> &sts) const {
    float costs = 0.0f;
    int cdemand;
    EACH_RES_PERIOD(
        cdemand = 0;
        EACH_JOB(if (sts[j] != UNSCHEDULED && sts[j] < t && t <= sts[j] + durations[j]) cdemand += demands(j, r))
        costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
    )
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

    EACH_RES_PERIOD(resRem(r,t) = capacities[r])
    vector<int> ess = earliestStartSchedule(resRem);

    float maxCosts = totalCosts(resRem);

    int minMs = Utils::max(makespan(ess), tkappa);

    auto sts = serialSGS(topOrder);
    int maxMs = makespan(sts);

    EACH_PERIOD(revenue[t] = static_cast<float>(
		(minMs == maxMs || t < minMs) ? maxCosts :
		(t > maxMs) ? 0.0f :
		maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

int ProjectWithOvertime::computeTKappa() const {
    int tkappa = 0;
    EACH_RES(
		float tkappar = 0.0f;
        EACH_JOB(tkappar += durations[j] * demands(j,r))
		tkappar /= static_cast<float>(capacities[r] + zmax[r]);
		tkappar = ceil(tkappar);
		tkappa = Utils::max(tkappa, static_cast<int>(tkappar));
    )
    return tkappa;
}

vector<int> ProjectWithOvertime::decisionTimesForResDevProblem(const vector<int>& sts, const vector<int>& ests, const vector<int>& lfts, const Matrix<int> &resRem, int j) const {
	int lstj = lfts[j] - durations[j];
    int estj = ests[j];

	vector<int> decisionTimes;

	while(true) {
		if(estj > lstj) return decisionTimes;
		if(enoughCapacityForJobWithBaseInterval(sts, ests, lfts, resRem, j, estj)) break;
		estj++;
	}

	while(true) {
		if(lstj < estj) return decisionTimes;
		if(enoughCapacityForJobWithBaseInterval(sts, ests, lfts, resRem, j, lstj)) break;
		lstj--;
	}

	decisionTimes.push_back(estj);

	for(int tau = estj+1; tau <= lstj-1; tau++) {
		EACH_JOBi(
			if(i != j && sts[i] != UNSCHEDULED
				&& (sts[i] + durations[i] == tau || tau + durations[j] == sts[i])
				&& enoughCapacityForJobWithBaseInterval(sts, ests, lfts, resRem, j, tau)) {
			decisionTimes.push_back(tau);
		})
	}

	if(estj < lstj)
		decisionTimes.push_back(lstj);

	return decisionTimes;
}

float ProjectWithOvertime::extensionCosts(const Matrix<int> &resRem, int j, int stj) const {
	float costs = 0.0f;
    EACH_RES(ACTIVE_PERIODS(j, stj, costs += Utils::max(0, demands(j,r) - resRem(r,tau)) * kappa[r]))
	return costs;
}

SGSResult ProjectWithOvertime::serialSGSWithOvertime(const vector<int> &order, bool robust) const {
	Matrix<int> resRem = normalCapacityProfile();

	vector<int> sts(numJobs, UNSCHEDULED), fts(numJobs, UNSCHEDULED);

    for (int k=0; k<numJobs; k++) {
		int job = robust ? chooseEligibleWithLowestIndex(sts, order) : order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);

        int t;
        for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);

        pair<int, float> bestT = make_pair(t, numeric_limits<float>::lowest());

        for(;; t++) {
            if(!enoughCapacityForJobWithOvertime(job, t, resRem))
                continue;

            Matrix<int> resRemTmp(resRem);
			vector<int> ftsTmp(fts);
            ftsTmp[job] = t;
            EACH_RES(ACTIVE_PERIODS(job, t, resRemTmp(r, tau) -= demands(job, r)))
            complementPartialWithSSGS(order, k+1, ftsTmp, resRemTmp, robust);

            float p = calcProfit(makespan(ftsTmp), resRemTmp);
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

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const {
    ACTIVE_PERIODS(job, t, EACH_RES(if(demands(job,r) > resRem(r,tau) + zmax[r]) return false))
    return true;
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, bool robust) const {
	Matrix<int> resRem = normalCapacityProfile();

    vector<int> sts(numJobs, UNSCHEDULED), fts(numJobs);
	
    for (int k=0; k<numJobs; k++) {
        int job = robust ? chooseEligibleWithLowestIndex(sts, order) : order[k];
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

SGSResult ProjectWithOvertime::serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust) const {
	Matrix<int> resRem = normalCapacityProfile();

    vector<int> sts(numJobs, UNSCHEDULED), fts(numJobs);
    for (int k=0; k<numJobs; k++) {
        int job = robust ? chooseEligibleWithLowestIndex(sts, order) : order[k];
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

bool ProjectWithOvertime::enoughCapacityForJobWithBaseInterval(const vector<int>& sts, const vector<int>& cests, const vector<int>& clfts, const Matrix<int> & resRem, int j, int stj) const {
	if(stj + durations[j] >= numPeriods) return false;

	for(int tau = stj + 1; tau <= stj + durations[j]; tau++) {
		for (int r = 0; r < numRes; r++) {
			int baseIntervalDemands = 0;

			for (int i = 0; i < numJobs; i++)
				if (sts[i] == UNSCHEDULED && tau >= clfts[i] - durations[i] + 1 && tau <= cests[i] + durations[i])
					baseIntervalDemands += demands(i, r);

			if (baseIntervalDemands + demands(j, r) > resRem(r, tau) + zmax[r])
				return false;
		}
	}

	return true;
}

pair<bool, SGSResult> ProjectWithOvertime::serialSGSWithDeadlineEarly(int deadline, const vector<int>& order) const {
	return serialSGSWithDeadline(deadline, order, [](int j, int count) { return 0; });
}

pair<bool, SGSResult> ProjectWithOvertime::serialSGSWithDeadlineLate(int deadline, const vector<int>& order) const {
	return serialSGSWithDeadline(deadline, order, [](int j, int count) { return count-1; });
}

pair<bool, SGSResult> ProjectWithOvertime::serialSGSWithDeadlineBeta(int deadline, const vector<int>& order, const vector<int>& beta) const {
	return serialSGSWithDeadline(deadline, order, [&beta](int j, int count) { return beta[j] > 0 ? 0 : count - 1; });
}

int ProjectWithOvertime::nthDecisionTimeWithMinCosts(int nth, vector<int> &decisionTimes, vector<float> &assocExtCosts, float minCosts) {
	int ctr = 0;
	for (int i = 0; i < decisionTimes.size(); i++) {
		if (assocExtCosts[i] == minCosts) {
			if(ctr == nth)
				return decisionTimes[i];

			ctr++;
		}
	}
	throw runtime_error("None has min costs!");
}

template<class Func>
pair<bool, SGSResult> ProjectWithOvertime::serialSGSWithDeadline(int deadline, const vector<int> &order, Func chooseIndex) const {
	Matrix<int> resRem = normalCapacityProfile();
	vector<int> sts(numJobs, UNSCHEDULED);

    for(int job : order) {
		vector<int> cests = earliestStartingTimesForPartial(sts);
		vector<int> clfts = latestFinishingTimesForPartial(sts, deadline);

		if(cests[job] > clfts[job] - durations[job]) {
            return make_pair(false, make_pair(sts, resRem));
        }

		// decision times for quasistable schedules
		vector<int> decisionTimes = decisionTimesForResDevProblem(sts, cests, clfts, resRem, job);

		int t = -1;

		if (!decisionTimes.empty()) {
			vector<float> assocExtCosts(decisionTimes.size());

			int i = 0;
			for (int dt : decisionTimes) {
				assocExtCosts[i] = extensionCosts(resRem, job, dt);
				i++;
			}

			float minCosts = *min(assocExtCosts.begin(), assocExtCosts.end());
			int minCount = count_if(assocExtCosts.begin(), assocExtCosts.end(), [minCosts](float c) { return c == minCosts; });

			int nth = chooseIndex(job, minCount);
			t = nthDecisionTimeWithMinCosts(nth, decisionTimes, assocExtCosts, minCosts);
		}

		if(t == -1) {
            return make_pair(false, make_pair(sts, resRem));
        }

		scheduleJobAt(job, t, sts, resRem);
    }

	return make_pair(true, make_pair(sts, resRem));
}

vector<int> ProjectWithOvertime::earliestStartingTimesForPartialRespectZmax(const vector<int> &sts, const Matrix<int> &resRem) const {
    vector<int> ests(numJobs), relaxedEsts = earliestStartingTimesForPartial(sts);

    transferAlreadyScheduled(ests, sts);

    for(int j : topOrder) {
        if(sts[j] != UNSCHEDULED) continue;
        ests[j] = 0;
        EACH_JOBi(if(adjMx(i, j)) ests[j] = Utils::max(ests[j], relaxedEsts[i] + durations[i]))

        int tau;
        for(tau = ests[j]; !enoughCapacityForJobWithOvertime(j, tau, resRem); tau++) ;
        ests[j] = tau;
    }

    return ests;
}


