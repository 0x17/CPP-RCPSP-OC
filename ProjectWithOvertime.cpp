﻿//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include <string>
#include <map>
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

float ProjectWithOvertime::totalCosts(const SGSResult& result) const {
	return totalCosts(result.resRem);
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

float ProjectWithOvertime::calcProfit(const SGSResult& result) const {
	return calcProfit(makespan(result.sts), result.resRem);
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

vector<int> ProjectWithOvertime::jobsWithDescendingStartingTimes(const vector<int>& sts) {
	vector<pair<int, int>> jobToSt(sts.size());
	for (int i = 0; i < sts.size(); i++)
		jobToSt[i] = make_pair(i, sts[i]);
	sort(jobToSt.begin(), jobToSt.end(), [](pair<int, int> &fst, pair<int, int> &snd) { return snd.second < fst.second; });
	vector<int> descSts(sts.size());
	for (int i = 0; i < sts.size(); i++)
		descSts[i] = jobToSt[i].first;
	return descSts;
}

list<int> ProjectWithOvertime::feasibleTimeWindowForJobInCompleteSchedule(int j, const vector<int>& sts, const vector<int>& fts, const Matrix<int>& resRem) const {
	list<int> feasTimes;
	int estj = computeLastPredFinishingTimeForPartial(fts, j);
	int lstj = computeFirstSuccStartingTimeForPartial(sts, j);

	for (int t = estj; t <= lstj; t++)
		if (enoughCapacityForJobWithOvertime(j, t, resRem))
			feasTimes.push_back(t);

	return feasTimes;
}

int ProjectWithOvertime::latestPeriodWithMinimalCosts(int j, const list<int>& feasTimes, const vector<int>& sts, const Matrix<int>& resRem) const {
	float minCosts = numeric_limits<float>::max();
	int latestPeriod = 0;

	for(int t : feasTimes) {
		float extCosts = extensionCosts(resRem, j, t);
		if(extCosts <= minCosts) {
			minCosts = extCosts;
			latestPeriod = t;
		}
	}

	return latestPeriod;
}

void ProjectWithOvertime::unscheduleJob(int j, vector<int>& sts, vector<int>& fts, Matrix<int>& resRem) const {
	unscheduleJob(j, sts[j], resRem);
	sts[j] = fts[j] = UNSCHEDULED;
}

void ProjectWithOvertime::unscheduleJob(int j, int stj, Matrix<int>& resRem) const {
	int ftj = stj + durations[j];
	for (int t = stj + 1; t <= ftj; t++)
		for (int r = 0; r < numRes; r++) resRem(r, t) += demands(j, r);
}


// FIXME: Finish this method
void ProjectWithOvertime::improvementStep(vector<int>& sts) {
	vector<int> fts(sts.size());
	for (int i = 0; i < sts.size(); i++) fts[i] = sts[i] + durations[i];

	vector<int> jobs = jobsWithDescendingStartingTimes(sts);
	Matrix<int> resRem = resRemForPartial(sts);

	// from latest to earliest job
	for(int j : jobs)
	{
		unscheduleJob(j, sts, fts, resRem);
		// compute resource-feasible subset of time window
		list<int> feasTimeWindow = feasibleTimeWindowForJobInCompleteSchedule(j, sts, fts, resRem);
		// choose latest period from set with minimal overtime costs
		int t = latestPeriodWithMinimalCosts(j, feasTimeWindow, sts, resRem);
		scheduleJobAt(j, t, sts, fts, resRem);
	}

	// vgl. F&O
}

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions(int ix) {
	setFromIndex(ix);
}

void ProjectWithOvertime::BorderSchedulingOptions::setFromIndex(int ix) {
	separateCrossover = ix == -1 ? false : Utils::int2bool((ix / 4) % 2);
	linked = ix == -1 ? false : Utils::int2bool((ix / 2) % 2);
	upper = ix == -1 ? false : Utils::int2bool(ix % 2);
}

int ProjectWithOvertime::heuristicMakespanUpperBound() const {
	static int ms = makespan(serialSGS(topOrder));
	return ms;
}

SGSResult ProjectWithOvertime::earlyOvertimeDeadlineOffsetSGS(const vector<int> &order, int deadlineOffset, bool robust) const {
	auto baseSchedule = serialSGS(order, zmax, robust);
	if(deadlineOffset <= 0) return baseSchedule;
    int baseMakespan = makespan(baseSchedule.sts);
	return delayWithoutOvertimeIncrease(order, baseSchedule.sts, baseSchedule.resRem, baseMakespan + deadlineOffset, robust);
}

SGSResult ProjectWithOvertime::delayWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust) const {
	vector<int> sts(baseSts);
	Matrix<int> resRem(baseResRem);
	vector<bool> unscheduled(numJobs, true);

	sts[numJobs - 1] = deadline;
	unscheduled[numJobs - 1] = false;

	for(int k = numJobs - 2; k >= 0; k--) {
		int j = robust ? chooseEligibleWithHighestIndex(unscheduled, order) : order[k];
		int baseStj = sts[j];
		int lstj = computeFirstSuccStartingTime(sts, j) - durations[j];
		unscheduleJob(j, sts[j], resRem);
		scheduleJobAt(j, latestCheapestPeriod(j, baseStj, lstj, resRem), sts, resRem);
		unscheduled[j] = false;
	}

	if(sts[0] > 0) {
		shiftScheduleLeftBy(sts[0], sts, resRem);
	}

	return { sts, resRem };
}

SGSResult ProjectWithOvertime::earlierWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, bool robust) const {
	vector<int> sts(baseSts);
	Matrix<int> resRem(baseResRem);
	vector<bool> unscheduled(numJobs, true);

	sts[0] = 0;
	unscheduled[0] = false;

	for (int k = 1; k < numJobs; k++) {
		int j = robust ? chooseEligibleWithLowestIndex(unscheduled, order) : order[k];
		int baseStj = sts[j];
		int estj = computeLastPredFinishingTime(sts, j);
		unscheduleJob(j, sts[j], resRem);
		scheduleJobAt(j, earliestCheapestPeriod(j, baseStj, estj, resRem), sts, resRem);
		unscheduled[j] = false;
	}

	return{ sts, resRem };
}

SGSResult ProjectWithOvertime::forwardBackwardWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust) const {
	auto result = delayWithoutOvertimeIncrease(order, baseSts, baseResRem, deadline, robust);
	vector<int> sts = result.sts;
	Matrix<int> resRem = result.resRem;
	return earlierWithoutOvertimeIncrease(order, sts, resRem, robust);
}

float ProjectWithOvertime::costsCausedByActivity(int j, int stj, const Matrix<int>& resRem) const {
	float costs = 0;
	ACTIVE_PERIODS(j, stj, EACH_RES(costs += max(demands(j, r) - resRem(r, tau), 0) * kappa[r]))
	return costs;
}

int ProjectWithOvertime::latestCheapestPeriod(int j, int baseStj, int lstj, const Matrix<int>& resRem) const {
	float baseOvertimeCosts = costsCausedByActivity(j, baseStj, resRem);
	for(int tau = lstj; tau > baseStj; tau--) {
		if(costsCausedByActivity(j, tau, resRem) <= baseOvertimeCosts) {
			return tau;
		}
	}
	return baseStj;
}

int ProjectWithOvertime::earliestCheapestPeriod(int j, int baseStj, int estj, const Matrix<int>& resRem) const {
	float baseOvertimeCosts = costsCausedByActivity(j, baseStj, resRem);
	for (int tau = estj; tau < baseStj; tau++) {
		if (costsCausedByActivity(j, tau, resRem) <= baseOvertimeCosts) {
			return tau;
		}
	}
	return baseStj;
}

// Allgemeine Utils::goldenSectionSearch methode
// Typ-Parameter: x-werte (deadline), y-werte (gewinn), y=g(f(x)), zugehöriger Zwischenwert f(x)
// f(x) => deadline zu plan
// g(x) => plan zu gewinn
SGSResult ProjectWithOvertime::goldenSectionSearchBasedOptimization(const vector<int>& order, bool robust) const {
	struct DeadlineProfitResultTriple {
		int deadline;
		float profit;
		SGSResult result;

		string toString() const {
			return "deadline=" + to_string(deadline) + ", profit=" + to_string(profit);
		}

	} lb, ub, a, b;	

	vector<int> zeroOc(numRes, 0);
	auto tminRes = serialSGS(order, zmax, robust);
	auto tmaxRes = serialSGS(order, zeroOc, robust);

	auto fillTripleFromSGSResult = [this](const SGSResult &res, DeadlineProfitResultTriple &t) {
		t.deadline = makespan(res.sts);
		t.profit = calcProfit(res);
		t.result = res;
	};

	fillTripleFromSGSResult(tminRes, lb);
	fillTripleFromSGSResult(tmaxRes, ub);

    double delta = (3.0 - sqrt(5.0)) / 2.0;

	auto iterateFBPassAndOutput = [&](SGSResult result, int deadline) {
		const vector<string> stepTypes = { "delay", "earlier" };
		for(int i=0; i<10; i++) {
			printf("deadline = %d, actual makespan = %d, Profit = %.2f, costs = %.2f, next step type = %s\n", deadline, makespan(result), calcProfit(result), totalCosts(result), stepTypes[i % 2].c_str());
			system("pause");
			result = (i % 2 == 0) ? delayWithoutOvertimeIncrease(order, result.sts, result.resRem, deadline, robust) : earlierWithoutOvertimeIncrease(order, result.sts, result.resRem, robust);
			if(!isScheduleFeasible(result.sts)) {
				LOG_W("INFEASIBLE SCHEDULE!");				
			}
		}
		return result;
	};

	auto updateTriple = [&](DeadlineProfitResultTriple &t) {
		auto baseResult = (t.deadline > a.deadline) ? a.result : lb.result;
		//t.result = delayWithoutOvertimeIncrease(order, baseResult.sts, baseResult.resRem, t.deadline, robust);
		t.result = iterateFBPassAndOutput(baseResult, t.deadline);
		t.profit = calcProfit(t.result);
	};

	a.deadline = lb.deadline + static_cast<int>(round(delta * (ub.deadline - lb.deadline)));
	b.deadline = lb.deadline + static_cast<int>(round((1.0 - delta) * (ub.deadline - lb.deadline)));
	updateTriple(a);
	updateTriple(b);
	
	while(true) {
		if(abs(lb.deadline - ub.deadline) <= 1) {
            if(ub.profit > lb.profit) lb = ub;
			break;
		}

		if (a.profit > b.profit) {
			ub = b;
			b = a;
			a.deadline = lb.deadline + static_cast<int>(round(delta * (ub.deadline - lb.deadline)));
			updateTriple(a);
		}
		else {
			lb = a;
			a = b;
			b.deadline = lb.deadline + static_cast<int>(round((1.0 - delta) * (ub.deadline - lb.deadline)));
			updateTriple(b);
		}
	}

	return lb.result;
}

bool ProjectWithOvertime::isScheduleResourceFeasible(const vector<int>& sts) const {
	return Project::isScheduleResourceFeasible(sts, zmax);
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

	return {sts, resRem};
}

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const {
    ACTIVE_PERIODS(job, t, EACH_RES(if(demands(job,r) > resRem(r,tau) + zmax[r]) return false))
    return true;
}

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions()
	: separateCrossover(false), linked(false), upper(false) {}

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions(bool _robust, bool _linked, bool _upper)
	: separateCrossover(_robust), linked(_linked), upper(_upper) {}

ProjectWithOvertime::PartialScheduleData::PartialScheduleData(ProjectWithOvertime const* p)
	: resRem(p->normalCapacityProfile()), sts(p->numJobs, p->UNSCHEDULED), fts(p->numJobs, p->UNSCHEDULED) {}

ProjectWithOvertime::ResidualData::ResidualData(ProjectWithOvertime const* p)
	: normal(p->normalCapacityProfile()),
	  overtime(p->numRes, p->numPeriods, [p](int r, int t) { return p->zmax[r]; })
{}

void ProjectWithOvertime::scheduleJobSeparateResiduals(int job, int t, int bval, PartialScheduleData& data, ResidualData& residuals) const {
	data.sts[job] = t;
	data.fts[job] = t + durations[job];

	ACTIVE_PERIODS(job, t, EACH_RES(
		int djr = demands(job, r);
		data.resRem(r, tau) -= djr;
		if(bval == 1) {
			if(djr > residuals.overtime(r, tau)) {
				residuals.normal(r, tau) -= (djr - residuals.overtime(r, tau));
				residuals.overtime(r, tau) = 0;
			} else residuals.overtime(r, tau) -= djr;
		}
		else residuals.normal(r, tau) -= djr;
	))
}

void ProjectWithOvertime::scheduleJobBorderLower(int job, int lastPredFinished, int bval, PartialScheduleData &data) const {
	int t;
	if(bval == 1) for(t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, data.resRem); t++);
	else for(t = lastPredFinished; !enoughCapacityForJob(job, t, data.resRem); t++);
	scheduleJobAt(job, t, data.sts, data.fts, data.resRem);
}

void ProjectWithOvertime::scheduleJobBorderUpper(int job, int lastPredFinished, int bval, PartialScheduleData& data, ResidualData& residuals) const {
	int t;
	if(bval == 1) for(t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, data.resRem); t++);
	else for(t = lastPredFinished; !enoughCapacityForJob(job, t, residuals.normal); t++);
	scheduleJobSeparateResiduals(job, t, bval, data, residuals);
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, BorderSchedulingOptions options, bool robust) const {
	ResidualData *residuals = nullptr;
	if(options.upper) {
		residuals = new ResidualData(this);
	}

	PartialScheduleData data(this);

    for (int k=0; k<numJobs; k++) {
        int job = robust ? chooseEligibleWithLowestIndex(data.sts, order) : order[k];
        int lastPredFinished = computeLastPredFinishingTime(data.fts, job);
		int bval = options.linked ? beta[k] : beta[job];
		if (!options.upper) scheduleJobBorderLower(job, lastPredFinished, bval, data);
		else scheduleJobBorderUpper(job, lastPredFinished, bval, data, *residuals);
    }

	if (residuals != nullptr)
		delete residuals;

	return{ data.sts, data.resRem };
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

	return{ sts, resRem };
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

SGSDeadlineResult ProjectWithOvertime::serialSGSWithDeadlineEarly(int deadline, const vector<int>& order) const {
	return serialSGSWithDeadline(deadline, order, [](int j, int count) { return 0; });
}

SGSDeadlineResult ProjectWithOvertime::serialSGSWithDeadlineLate(int deadline, const vector<int>& order) const {
	return serialSGSWithDeadline(deadline, order, [](int j, int count) { return count-1; });
}

SGSDeadlineResult ProjectWithOvertime::serialSGSWithDeadlineBeta(int deadline, const vector<int>& order, const vector<int>& beta) const {
	return serialSGSWithDeadline(deadline, order, [&beta](int j, int count) { return beta[j] > 0 ? 0 : count - 1; });
}

SGSDeadlineResult ProjectWithOvertime::serialSGSWithDeadlineTau(int deadline, const vector<int>& order, const vector<float>& tau) const {
	return serialSGSWithDeadline(deadline, order, [&tau](int j, int count) { return static_cast<int>(round(static_cast<float>(count - 1) * (1.0f - tau[j]))); });
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
SGSDeadlineResult ProjectWithOvertime::serialSGSWithDeadline(int deadline, const vector<int> &order, Func chooseIndex) const {
	Matrix<int> resRem = normalCapacityProfile();
	vector<int> sts(numJobs, UNSCHEDULED);

    for(int job : order) {
		vector<int> cests = earliestStartingTimesForPartial(sts);
		vector<int> clfts = latestFinishingTimesForPartial(sts, deadline);

		if(cests[job] > clfts[job] - durations[job]) {
			return{ false, sts, resRem }; //  make_pair(false, make_pair(sts, resRem));
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
			int minCount = static_cast<int>(count_if(assocExtCosts.begin(), assocExtCosts.end(), [minCosts](float c) { return c == minCosts; }));

			int nth = chooseIndex(job, minCount);
			t = nthDecisionTimeWithMinCosts(nth, decisionTimes, assocExtCosts, minCosts);
		}

		if(t == -1) {
			return{ false, sts, resRem };
        }

		scheduleJobAt(job, t, sts, resRem);
    }

	return{ true, sts, resRem };
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