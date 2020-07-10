//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>

#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/string/join.hpp>

#include "ProjectWithOvertime.h"
#include "GurobiSolver.h"

using namespace std;

ProjectWithOvertime::ProjectWithOvertime(JsonWrap _obj) : Project(_obj) {
	auto obj = _obj.obj;
    revenue = JsonUtils::extractNumberArrayFromObj(obj, "u");
    kappa = JsonUtils::extractNumberArrayFromObj(obj, "kappa");
    zmax = JsonUtils::extractIntArrayFromObj(obj, "zmax");
}

ProjectWithOvertime::ProjectWithOvertime(const string &filename) :
	ProjectWithOvertime(boost::filesystem::path(filename).stem().string(), Utils::readLines(filename)) {}

ProjectWithOvertime::ProjectWithOvertime(const string& projectName, const string& contents) :
	ProjectWithOvertime(projectName, Utils::splitLines(contents)) {}

ProjectWithOvertime::ProjectWithOvertime(const string& projectName, const vector<string>& lines) :
	Project(projectName, lines),
	zmax(numRes),
	zzero(numRes, 0),
	kappa(numRes),
	revenue(numPeriods),
	revenueExtrapolated(numPeriods) {
	eachRes([&](int r) {
		zmax[r] = capacities[r] / 2; // FIXME: should be using floor operator here!
		kappa[r] = 0.5f;
	});
	computeRevenueFunction();
	computeExtrapolatedRevenueFunction();
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

float ProjectWithOvertime::calcProfitForPartial(const std::vector<int>& sts) const {
	return revenueExtrapolated[makespanOfPartial(sts)] - totalCostsForPartial(sts);
}

float ProjectWithOvertime::calcProfit(int makespan, const Matrix<int>& resRem) const {
	return revenue[makespan] - totalCosts(resRem);
}

float ProjectWithOvertime::calcProfit(const SGSResult& result) const {
	return calcProfit(makespan(result.sts), result.resRem);
}

void ProjectWithOvertime::computeRevenueFunction() {
	auto ess = earliestStartSchedule();

	// minMs previously defined as: Utils::max(makespan(ess), computeTKappa());
	int minMs = makespan(ess);
    int maxMs = makespan(serialSGS(topOrder));

	float maxCosts = totalCosts(ess);

    EACH_PERIOD(revenue[t] = static_cast<float>(
		(minMs >= maxMs || t < minMs) ? maxCosts :
		(t > maxMs) ? 0.0f :
		maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

void ProjectWithOvertime::computeExtrapolatedRevenueFunction() {
	const auto ess = earliestStartSchedule();

	const int minMs = makespan(ess);
	const int maxMs = makespan(serialSGS(topOrder));

	const float maxCosts = totalCosts(ess);

	const auto extrapolatedLine = [&](int t) {
		return maxCosts / (maxMs - minMs) * (maxMs - t);
	};

	EACH_PERIOD(revenueExtrapolated[t] = static_cast<float>(
		minMs >= maxMs ? maxCosts :
		t < minMs ? extrapolatedLine(t) :
		t > maxMs ? 0.0f :
		maxCosts - maxCosts / pow(maxMs - minMs, 2) * pow(t - minMs, 2)))
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

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions(int ix) {
	setFromIndex(ix);
}

void ProjectWithOvertime::BorderSchedulingOptions::setFromIndex(int ix) {
	separateCrossover = Utils::int2bool((ix / 4) % 2);
	assocIndex = Utils::int2bool((ix / 2) % 2);
	upper = Utils::int2bool(ix % 2);
}

int ProjectWithOvertime::heuristicMakespanUpperBound() const {
	static int ms = makespan(serialSGS(topOrder));
	return ms;
}

SGSResult ProjectWithOvertime::forwardBackwardDeadlineOffsetSGS(const vector<int> &order, int deadlineOffset, bool robust) const {
	auto baseSchedule = serialSGS(order, zmax, robust);
	if(deadlineOffset <= 0) return baseSchedule;
    int baseMakespan = makespan(baseSchedule.sts);
	auto res = forwardBackwardIterations(order, baseSchedule, baseMakespan + deadlineOffset, boost::optional<int>(), robust);
	res.numSchedulesGenerated++;
	return res;
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
		unscheduleJob(j, sts, resRem);
		scheduleJobAt(j, latestCheapestFeasiblePeriod(j, baseStj, lstj, resRem), sts, resRem);
		unscheduled[j] = false;
	}

	if(sts[0] > 0) {
		shiftScheduleLeftBy(sts[0], sts, resRem);
	}

	return { sts, resRem };
}

SGSResult ProjectWithOvertime::earlierWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, bool robust) const {
	vector<int> sts(baseSts);
	vector<int> fts = stsToFts(sts);
	Matrix<int> resRem(baseResRem);
	vector<bool> unscheduled(numJobs, true);

	sts[0] = fts[0] = 0;
	unscheduled[0] = false;

	for (int k = 1; k < numJobs; k++) {
		int j = robust ? chooseEligibleWithLowestIndex(unscheduled, order) : order[k];
		int baseStj = sts[j];
		int estj = computeLastPredFinishingTime(fts, j);
		unscheduleJob(j, sts, fts, resRem);
		scheduleJobAt(j, earliestCheapestFeasiblePeriod(j, baseStj, estj, resRem), sts, fts, resRem);
		unscheduled[j] = false;
	}

	return{ sts, resRem };
}

boost::optional<float> ProjectWithOvertime::costsAndFeasibilityCausedByActivity(int j, int stj, const Matrix<int>& resRem) const {
	float costs = 0.0f;
	ACTIVE_PERIODS(j, stj,
		EACH_RES(
			if (demands(j, r) > resRem(r, tau) + zmax[r]) return boost::optional<float>();
			costs += boost::algorithm::clamp(demands(j, r) - resRem(r, tau), 0, demands(j, r)) * kappa[r];
	))
	return boost::optional<float>(costs);
}

int ProjectWithOvertime::latestCheapestFeasiblePeriod(int j, int baseStj, int lstj, const Matrix<int>& resRem) const {
	float baseOvertimeCosts = *costsAndFeasibilityCausedByActivity(j, baseStj, resRem);
	for(int tau = lstj; tau > baseStj; tau--) {
		auto optCosts = costsAndFeasibilityCausedByActivity(j, tau, resRem);
		if(optCosts && *optCosts <= baseOvertimeCosts) {
			return tau;
		}
	}
	return baseStj;
}

int ProjectWithOvertime::earliestCheapestFeasiblePeriod(int j, int baseStj, int estj, const Matrix<int>& resRem) const {
	float baseOvertimeCosts = *costsAndFeasibilityCausedByActivity(j, baseStj, resRem);
	for (int tau = estj; tau < baseStj; tau++) {
		auto optCosts = costsAndFeasibilityCausedByActivity(j, tau, resRem);
		if (optCosts && *optCosts <= baseOvertimeCosts) {
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
	int scheduleCount = 0;

	struct DeadlineProfitResultTriple {
		int deadline;
		float profit;
		SGSResult result;

		string toString() const {
			return "deadline=" + std::to_string(deadline) + ", profit=" + std::to_string(profit);
		}

	} lb, ub, a, b;



	auto tminResBase = serialSGS(order, zmax, robust);
	auto tmaxResBase = serialSGS(order, zzero, robust);

	auto tminRes = forwardBackwardIterations(order, tminResBase, makespan(tminResBase));
	auto tmaxRes = forwardBackwardIterations(order, tmaxResBase, makespan(tmaxResBase));

	scheduleCount += 2;

	auto fillTripleFromSGSResult = [this](const SGSResult &res, DeadlineProfitResultTriple &t) {
		t.deadline = makespan(res.sts);
		t.profit = calcProfit(res);
		t.result = res;
	};
	
	fillTripleFromSGSResult(tminRes, lb);
	fillTripleFromSGSResult(tmaxRes, ub);

	// schedule without overtime already shorter? (rare edge case)
	if(ub.deadline < lb.deadline) {
		return ub.result;
	}

    double delta = (3.0 - sqrt(5.0)) / 2.0;

	auto updateTriple = [&](DeadlineProfitResultTriple &t) {
		auto baseResult = (t.deadline > a.deadline) ? a.result : lb.result;
		t.result = forwardBackwardIterations(order, baseResult, t.deadline, boost::optional<int>(), robust);
		t.profit = calcProfit(t.result);
		scheduleCount += t.result.numSchedulesGenerated;
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

	lb.result.numSchedulesGenerated = scheduleCount;
	return lb.result;
}

std::map<int, std::pair<int, float>> ProjectWithOvertime::heuristicProfitsAndActualMakespanForRelevantDeadlines(const vector<int> &order) const {
	auto tminRes = serialSGS(order, zmax);
	auto tmaxRes = serialSGS(order, zzero);
	int lb = makespan(tminRes.sts);
	int ub = makespan(tmaxRes.sts);
	std::map<int, std::pair<int, float>> profitForMakespan;
	for(int t=lb; t <= ub; t++) {
		auto result = forwardBackwardIterations(order, tminRes, t);
		int actualMakespan = makespan(result);
		float delayedProfit = calcProfit(result);
		profitForMakespan[t] = std::make_pair(actualMakespan, delayedProfit);
	}
	return profitForMakespan;
}

json11::Json ProjectWithOvertime::to_json() const {
	const auto rocObj = json11::Json::object {
		{"u", revenue},
		{"kappa", kappa},
		{"zmax", zmax}
	};
	return JsonUtils::mergeObjects(Project::to_json(), rocObj);
}

void ProjectWithOvertime::from_json(const json11::Json& obj) {
	Project::from_json(obj);
	revenue = JsonUtils::extractNumberArrayFromObj(obj, "u");
	kappa = JsonUtils::extractNumberArrayFromObj(obj, "kappa");
	zmax = JsonUtils::extractIntArrayFromObj(obj, "zmax");
}

std::vector<int> ProjectWithOvertime::serialOptimalSubSGS(const std::vector<int>& orderInducedPartitions, int partitionSize) const {	
	std::vector<int> sts(numJobs, UNSCHEDULED), nextPartition(partitionSize, -1);

#ifndef DISABLE_GUROBI
	static GurobiSolverBase::Options opts;
	static GurobiSubprojectSolver solver(*this, opts);

	const int numPartitions = ceil((float)orderInducedPartitions.size() / (float)partitionSize);
	for(int p = 0; p < numPartitions; p++) {
		for(int j=0; j<partitionSize; j++) {
			int ix = p*partitionSize + j;
			if(ix >= orderInducedPartitions.size()) {
				nextPartition.resize(j);
				break;
			}
			nextPartition[j] = orderInducedPartitions[ix];
		}

		solver.setupModelForSubproject(sts, nextPartition, p == 0);
		solver.supplyWithMIPStart(complementPartialWithSpecificJobsUsingSSGS(sts, nextPartition), nextPartition);
		sts = solver.solve().sts;
	}
#endif

	return sts;
}

std::vector<int> ProjectWithOvertime::serialOptimalSubSGSWithPartitionList(const std::vector<int> &partitionList) const {
	const int numPartitions = *max_element(partitionList.begin(), partitionList.end()) + 1;
	const int partitionSize = count(partitionList.begin(), partitionList.end(), 0);

	std::vector<int> sts(numJobs, UNSCHEDULED), nextPartition(partitionSize, -1);
	
#ifndef DISABLE_GUROBI	
	static GurobiSolverBase::Options opts;
	static GurobiSubprojectSolver solver(*this, opts);

	//Stopwatch sw;
	//sw.start();

	for(int p=0; p<numPartitions; p++) {
		int ctr = 0;
		for(int j=0; j< numJobs; j++) {
			if(partitionList[j] == p) {
				nextPartition[ctr++] = j;
			}
		}

		solver.setupModelForSubproject(sts, nextPartition, p == 0);
		solver.supplyWithMIPStart(complementPartialWithSpecificJobsUsingSSGS(sts, nextPartition), nextPartition);

		//cout << "Time required setup = " << sw.lookAndReset() << endl;

		// Do problem specific partial branch and bound procedure here? what is a good UB for any choice?
		// Alternatively: Modified model with 'as early as possible without overtime increase' as objective
		sts = solver.solve().sts;

		//cout << "Time required MIP solve = " << sw.lookAndReset() << endl;
	}
#endif

	return sts;
}

bool ProjectWithOvertime::isPartitionListFeasible(const std::vector<int>& partitionList, int partitionSize) const {
	int partitionCount = static_cast<int>(ceil(numJobs / partitionSize));

	for (int pix = 0; pix < partitionCount - 1; pix++)
		if (partitionSize != count(partitionList.begin(), partitionList.end(), pix))
			return false;

	for (int i = 0; i < numJobs; i++)
		for (int j = 0; j < numJobs; j++)
			if (adjMx(i,j) && partitionList[i] > partitionList[j]) {
				return false;
			}
	
	return true;
}


bool ProjectWithOvertime::isScheduleResourceFeasible(const vector<int>& sts) const {
	return Project::isScheduleResourceFeasible(sts, zmax);
}

SGSResult ProjectWithOvertime::serialSGSWithOvertime(const vector<int> &order, bool robust) const {
	Matrix<int> resRem = normalCapacityProfile();

	vector<int> sts(numJobs, UNSCHEDULED), fts(numJobs, UNSCHEDULED);

    for (int k=0; k<numJobs; k++) {
		int job = robust ? chooseEligibleWithLowestIndex(sts, order) : order[k];
        int lastPredFinished = computeLastPredFinishingTime(fts, job);

        int t;
        for (t = lastPredFinished; !enoughCapacityForJobWithOvertime(job, t, resRem); t++);

        std::pair<int, float> bestT = std::make_pair(t, std::numeric_limits<float>::lowest());

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

	return {sts, resRem, 1};
}

SGSResult ProjectWithOvertime::serialSGSWithOvertimeWithForwardBackwardImprovement(const vector<int>& order, bool robust) const {
	SGSResult res = serialSGSWithOvertime(order, robust);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>(), robust);
}

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const {
    ACTIVE_PERIODS(job, t, EACH_RES(if(demands(job,r) > resRem(r,tau) + zmax[r]) return false))
    return true;
}

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions()
	: separateCrossover(false), assocIndex(false), upper(false) {}

ProjectWithOvertime::BorderSchedulingOptions::BorderSchedulingOptions(bool _separateCrossover, bool _assocIndex, bool _upper)
	: separateCrossover(_separateCrossover), assocIndex(_assocIndex), upper(_upper) {}

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

SGSResult ProjectWithOvertime::serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, const BorderSchedulingOptions &options, bool robust) const {
	std::unique_ptr<ResidualData> residuals = nullptr;
	if(options.upper) {
		residuals = std::make_unique<ResidualData>(this);
	}

	PartialScheduleData data(this);

    for (int k=0; k<numJobs; k++) {
        int job = robust ? chooseEligibleWithLowestIndex(data.sts, order) : order[k];
        int lastPredFinished = computeLastPredFinishingTime(data.fts, job);
		int bval = options.assocIndex ? beta[k] : beta[job];
		if (!options.upper) scheduleJobBorderLower(job, lastPredFinished, bval, data);
		else scheduleJobBorderUpper(job, lastPredFinished, bval, data, *residuals);
    }

	return { data.sts, data.resRem, 1 };
}

// (lambda, beta)
SGSResult ProjectWithOvertime::serialSGSTimeWindowBordersWithForwardBackwardImprovement(const vector<int>& order, const vector<int>& beta, const BorderSchedulingOptions &options, bool robust) const {
	SGSResult res = serialSGSTimeWindowBorders(order, beta, options, robust);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>(), robust);
}

// (lambda, zr)
SGSResult ProjectWithOvertime::serialSGSWithForwardBackwardImprovement(const vector<int>& order, const vector<int>& z, bool robust) const {
	SGSResult res = serialSGS(order, z, robust);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>(), robust);
}

// (lambda, zrt)
SGSResult ProjectWithOvertime::serialSGSWithForwardBackwardImprovement(const vector<int>& order, const Matrix<int>& z, bool robust) const {
	SGSResult res = serialSGS(order, z, robust);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>(), robust);
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

	return{ sts, resRem, 1 };
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

SGSResult ProjectWithOvertime::forwardBackwardIterations(const vector<int> &order, SGSResult result, int deadline, boost::optional<int> numIterations, bool robust) const {
	/*auto checkAndOutput = [&](const SGSResult &result, int deadline, int nextStepType) {
		const vector<string> stepTypes = { "delay", "earlier" };
		if(!isSchedulePrecedenceFeasible(result.sts))
			LOG_W("Precedence infeasible!");

		if(!isScheduleResourceFeasible(result.sts))
			LOG_W("Resource infeasible!");
		printf("deadline = %d, actual makespan = %d, Profit = %.2f, costs = %.2f, next step type = %s\n", deadline, makespan(result), calcProfit(result), totalCosts(result), stepTypes[nextStepType % 2].c_str());
		system("pause");
	};*/

	const int DEADLINE_IMPROVEMENT_TOLERANCE = 0;
	float lastCosts = totalCosts(result.resRem);
	int i;
	for(i=0; numIterations ? i < *numIterations : true; i++) {
		//checkAndOutput(result, deadline, i);
		result = (i % 2 == 0) ? delayWithoutOvertimeIncrease(order, result.sts, result.resRem, deadline, robust) : earlierWithoutOvertimeIncrease(order, result.sts, result.resRem, robust);
		const float currentCosts = totalCosts(result.resRem);
		if (std::abs(currentCosts - lastCosts) <= DEADLINE_IMPROVEMENT_TOLERANCE) {
			//checkAndOutput(result, deadline, i);
			i++;
			break;
		}
		lastCosts = currentCosts;
	}
	result.numSchedulesGenerated += i;
	return result;
}

SGSResult ProjectWithOvertime::serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(const vector<int> &order, const vector<float> &tau, bool robust) const {
	SGSResult res = serialSGSTimeWindowArbitrary(order, tau, robust);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>(), robust);
}

SGSResult ProjectWithOvertime::serialSGSWithRandomKeyAndFBI(const std::vector<float> &rk, const Matrix<int> &z) const {
	const SGSResult res = serialSGSWithRandomKey(rk, z);
	const vector<int> order = scheduleToActivityList(res.sts);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>());
}

SGSResult ProjectWithOvertime::serialSGSWithRandomKeyAndFBI(const std::vector<float>& rk, const std::vector<int>& z) const {
	const SGSResult res = serialSGSWithRandomKey(rk, z);
	const vector<int> order = scheduleToActivityList(res.sts);
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>());
}

SGSResult ProjectWithOvertime::serialOptimalSubSGSAndFBI(const std::vector<int> &orderInducedPartitions, int partitionSize, bool robust) const {
	const vector<int> sts = serialOptimalSubSGS(robust ? permutationToActivityList(orderInducedPartitions) : orderInducedPartitions, partitionSize);
	const Matrix<int> resRem = resRemForPartial(sts);
	const SGSResult res = { sts, resRem, 1 };
	return forwardBackwardIterations(orderInducedPartitions, res, makespan(res), boost::optional<int>());
}

SGSResult ProjectWithOvertime::serialOptimalSubSGSWithPartitionListAndFBI(const std::vector<int> &partitionList) const {
	const vector<int> sts = serialOptimalSubSGSWithPartitionList(partitionList);
	const vector<int> order = scheduleToActivityList(sts);
	const Matrix<int> resRem = resRemForPartial(sts);
	const SGSResult res = { sts, resRem, 1 };
	return forwardBackwardIterations(order, res, makespan(res), boost::optional<int>());
}

SGSResult ProjectWithOvertime::parallelSGSWithForwardBackwardImprovement(const std::vector<int>& order, const Matrix<int>& z) const {
	const auto res = parallelSGS(order, z);
	return forwardBackwardIterations(order, res, makespan(res));
}

SGSResult ProjectWithOvertime::parallelSGSWithForwardBackwardImprovement(const std::vector<int>& order, const std::vector<int>& z) const {
	const auto sts = parallelSGS(order, z);
	const auto resRem = resRemForPartial(sts);
	const SGSResult res = { sts, resRem, 1 };
	return forwardBackwardIterations(order, res, makespan(res));
}

SGSResult ProjectWithOvertime::parallelSGSWithForwardBackwardImprovement(const std::vector<int>& order) const {
	const auto sts = parallelSGS(order);
	const auto resRem = resRemForPartial(sts);
	const SGSResult res = {sts, resRem, 1};
	return forwardBackwardIterations(order, res, makespan(res));
}

struct MIPFormulationMetrics {
	string formulationName;
	int numBinaryVars, numContinuousVars, numConstraints;

	map<string, float> toDict() const;
};

map<string, float> MIPFormulationMetrics::toDict() const {
	return {
		{"numBinaryVars_" + formulationName, numBinaryVars},
		{"numContinuousVars" + formulationName, numContinuousVars},
		{"numConstraints" + formulationName, numConstraints}
	};
}

MIPFormulationMetrics computeMipFormulationMetricsForKopCt1(const ProjectWithOvertime& p) {
	MIPFormulationMetrics res = {};
	res.formulationName = "KopCt1";

	// 1. C, B, G
	const Matrix<char> C = Utils::transitiveClosure(p.adjMx);
	const Matrix<char> B(p.numJobs, p.numJobs, [&p](int i, int j) {
		return Utils::any([&p,i,j](int r) { return p.demands(i, r) > 0 && p.demands(j, r) > 0; }, 0, p.numRes);
	});
	const Matrix<char> G(p.numJobs, p.numJobs, [&p](int i, int j) {
		return Utils::any([&p, i, j](int r) { return p.demands(i, r) + p.demands(j, r) > p.capacities[r]; }, 0, p.numRes);
	});
	// 2. D
	const Matrix<char> D(p.numJobs, p.numJobs, [&p, &C](int i, int j) {
		return (p.lfts[i] <= p.ests[j]) && !C(i, j);
	});
	// 3. K
	const Matrix<char> K(p.numJobs, p.numJobs, [&C, &D](int i, int j) {
		return C(i, j) || D(i, j);
	});
	// 4. Kprime
	const Matrix<char> Kprime(p.numJobs, p.numJobs, [&K](int i, int j) {
		return K(i,j) || K(j,i);
	});
	// 5. S, P
	const Matrix<char> S(p.numJobs, p.numJobs, [&G, &Kprime](int i, int j) {
		return G(i,j) && !Kprime(i,j);
	});
	const Matrix<char> P(p.numJobs, p.numJobs, [&B, &G, &Kprime](int i, int j) {
		return B(i,j) && !G(i,j) && !Kprime(i,j);
	});

	const Matrix<char> one(p.numJobs, p.numJobs, 1);

	const vector<int> numResUsed = Utils::constructVector<int>(p.numJobs, [&p](int j) {
		return Utils::sum([&p,j](int r) { return p.demands(j, r) > 0 ? 1 : 0; }, 0, p.numRes);
	});

	const int pdiffcount = Utils::countPred(P, [](int i, int j) { return i != j; });

	res.numConstraints = p.numJobs - 1
		+ Utils::sum(K)
		+ Utils::countPred(S, [](int i, int j) { return i != j; })
		+ Utils::countPred(one, [&Kprime](int i, int j) { return i > j && !Kprime(i, j); })
		+ Utils::countPred(P, [](int i, int j) { return i > j; }) * 2
		+ pdiffcount * 2
	    + Utils::sum(numResUsed) 
		+ Utils::countPred(P, [&p](int i, int j) { return i != j && p.lsts[j] <= p.ests[i]; });

	res.numContinuousVars = 2 * (p.numJobs - 1);

	const int numX = pdiffcount;
	const int numZ = Utils::countPred(one, [&B, &G, &Kprime](int i, int j) { return (B(i, j) || G(i, j)) && !Kprime(i, j) && i != j; });

	res.numBinaryVars = numX + numZ;

	return res;
}

MIPFormulationMetrics computeMipFormulationMetricsForKopDt2(const ProjectWithOvertime& p) {
	MIPFormulationMetrics res = {};
	res.formulationName = "KopDt2";

	const int lsMinusEsSum = Utils::sum([&p](int j) { return p.lsts[j] - p.ests[j];  }, 0, p.numJobs);
	const int lfMinusEsSum = Utils::sum([&p](int j) { return p.lfts[j] - p.ests[j] - 1;  }, 0, p.numJobs);

	const int numY = (p.numJobs - 1)*lsMinusEsSum;
	const int numW = (p.numJobs - 2)*lfMinusEsSum;

	res.numBinaryVars = numY + numW;
	res.numContinuousVars = 1;

	int numEdges = 0;
	p.adjMx.foreach([&numEdges](int i, int j, char v) {
		numEdges += v;
	});
	res.numConstraints = numEdges + (p.numPeriods+1)*p.numRes + p.numJobs-1 + lfMinusEsSum;

	return res;
}

ProjectCharacteristics ProjectWithOvertime::collectCharacteristics(const boost::optional<std::map<std::string, float>> additionalCharacteristics) const {
	vector<int> zeroOc(numRes, 0);
	const auto zbalanced = Utils::constructVector<int>(numRes, [this](int r) { return round(zmax[r] / 2.0f); });

	const auto tminSGSSchedule = serialSGS(topOrder, zmax);
	const float cmaxSGS = totalCosts(tminSGSSchedule);
	const int tminSGS = makespan(tminSGSSchedule);

	SGSResult ess = earliestStartSchedule();
	const float cmax = totalCosts(ess);
	const int tmin = makespan(ess);
	const int tmax = makespan(serialSGS(topOrder, zeroOc));

	const auto computeNetworkComplexity = [this]() {
		// FIXME: Remove redundant arcs?!!
		return (float)Utils::sum(adjMx) / (float)numJobs;
	};

	const auto computeResourceFactor = [this]() {
		int strictlyPositiveDemandCount = 0;
		eachJobResConst([this, &strictlyPositiveDemandCount](int j, int r) {
			strictlyPositiveDemandCount += demands(j, r) > 0 ? 1 : 0;
		});

		return (float)strictlyPositiveDemandCount / (float)(numJobs * numRes);
	};

	const auto computeResourceStrength = [&]() {
		vector<float> results(numRes+1);
		vector<int> maxActivityConsumptions(numRes);
		eachJobResConst([this, &maxActivityConsumptions](int j, int r) {
			maxActivityConsumptions[r] = max(demands(j, r), maxActivityConsumptions[r]);
		});

		vector<int> mostNeg(numRes, numeric_limits<int>::max());
		ess.resRem.foreach([&mostNeg](int r, int t, int val) {
			mostNeg[r] = min(val, mostNeg[r]);
		});
		const auto maxCumulativeConsumptionsESS = Utils::constructVector<int>(numRes, [this, &mostNeg](int r) {
			return max(-mostNeg[r], 0) + capacities[r];
		});

		float sumRs = 0;
		eachResConst([&](int r) {
			results[r] = (maxCumulativeConsumptionsESS[r] - maxActivityConsumptions[r] == 0) ? 1.0f : (float)(capacities[r] - maxActivityConsumptions[r]) / (float)(maxCumulativeConsumptionsESS[r] - maxActivityConsumptions[r]);
			sumRs += results[r];
		});

		results[numRes] = sumRs / (float)numRes;
		return results;
	};

	const auto computeOrderStrength = [&]() {
		return static_cast<float>(Utils::sum(Utils::transitiveClosure(adjMx))) / (static_cast<float>(numJobs*numJobs-numJobs)/2.0f);
	};

	const auto computeResourceConstrainednessForRes = [&](int r) {
		int cumDem = 0;
		int usageCount = 0;
		eachJobConst([&](int j) {
			cumDem += demands(j, r);
			usageCount += demands(j,r) > 0 ? 1 : 0;
		});
		return (float)cumDem / (float)usageCount / (float)capacities[r];
	};

	const auto computeResourceConstrainedness = [&]() {
		float rc = 0.0f;
		eachResConst([&](int r){  rc += computeResourceConstrainednessForRes(r); });
		rc /= static_cast<float>(numRes);
		return rc;
	};

	const int minCapacity = *std::min_element(capacities.begin(), capacities.end());
	const int maxCapacity = *std::max_element(capacities.begin(), capacities.end());
	const float avgCapacity = Utils::average(capacities);
	const float avgDuration = Utils::average(durations);
	const float avgBranchFactor = Utils::sum(adjMx);

	const float revWidth = tmax-tmin;
	const float revSlope = revWidth > 0.0f ? (revenue[tmin]-revenue[tmax])/revWidth : 0.0f;

	const auto balancedSchedule = serialSGS(topOrder, zbalanced);
	const float cmaxBalanced = totalCosts(balancedSchedule);
	const int tminBalanced = makespan(balancedSchedule);

	const vector<int> totalCapacities = Utils::constructVector<int>(numRes, [this](int r) { return capacities[r] + zmax[r]; });
	const float avgTotalCapacity = Utils::average(totalCapacities);

	const auto numPeriodsWithOvertime = [this](const SGSResult &res) {
		float nperiods = 0.0f;
		eachPeriodConst([this, &res, &nperiods](int t) {
			for(int r=0; r<numRes; r++) {
				if (res.resRem(r, t) < 0) {
					nperiods++;
					break;
				}
			}
		});
		return nperiods;
	};

	const auto numJobsInOvertimePeriods = [this](const SGSResult &res) {
		vector<bool> jobInOC(numJobs, false);

		eachPeriodConst([this, &res, &jobInOC](int t) {
			for(int r=0; r<numRes; r++) {
				if (res.resRem(r, t) < 0) {
					for(int j=0; j<numJobs; j++) {
						if (isJobActiveInPeriod(res.sts, j, t)) {
							jobInOC[j] = true;
						}
					}
					break;
				}
			}
		});

		float njobs = 0.0f;
		for(bool indic : jobInOC) {
			if(indic) njobs++;
		}
		return njobs;
	};

	const auto averageResidualCapacity = [this](const SGSResult &res) {
		float accum = 0.0f;
		eachResPeriodConst([this, &accum, &res](int r, int t) {
			accum += max(0, res.resRem(r, t));
		});
		return accum / static_cast<float>(numRes * numPeriods);
	};

	vector<float> rsResults = computeResourceStrength();

	MIPFormulationMetrics ct1Metrics = computeMipFormulationMetricsForKopCt1(*this);
	MIPFormulationMetrics dt2Metrics = computeMipFormulationMetricsForKopDt2(*this);

	map<string, float> charMap = {
			{"njobs", numJobs},
			{"nres", numRes},
			{"nperiods", numPeriods},
			{"cmax", cmax},
			{"cmaxSGS", cmaxSGS},
		    {"tminSGS", tminSGS},
			{"tmin", tmin},
			{"tmax", tmax},
			{"nc", computeNetworkComplexity()},
			{"rf", computeResourceFactor()},
			{"rs", rsResults[rsResults.size()-1]},
			{"os", computeOrderStrength()},
			{"rc", computeResourceConstrainedness()},
			{"minCap", minCapacity},
			{"maxCap", maxCapacity},
			{"avgCap", avgCapacity},
			{"avgDur", avgDuration},
			{"avgBranch", avgBranchFactor},
			{"revWidth", revWidth},
			{"revSlope", revSlope},
			{"cmaxBalanced", cmaxBalanced},
			{"tminBalanced", tminBalanced},
			{"avgTotalCap", avgTotalCapacity},
			{"nperiodsOCESS", numPeriodsWithOvertime(ess)},
			{"nperiodsOCSGSzmax", numPeriodsWithOvertime(tminSGSSchedule)},
			{"njobsOCESS", numJobsInOvertimePeriods(ess)},
			{"njobsOCSGSzmax", numJobsInOvertimePeriods(tminSGSSchedule)},
			{"avgResESS", averageResidualCapacity(ess)},
			{"avgResSGSzmax", averageResidualCapacity(tminSGSSchedule)}
	};

	const auto extendCharMapWithDict = [&charMap](const map<string, float> &dict) {
		for (const auto &pair : dict) {
			charMap.insert(pair);
		}
	};

	for(int r=0; r<numRes; r++) {
		charMap.insert(make_pair("rc"+to_string(r), computeResourceConstrainednessForRes(r)));
		charMap.insert(make_pair("rs"+to_string(r), rsResults[r]));
	}

	if(additionalCharacteristics) {
		extendCharMapWithDict(*additionalCharacteristics);
	}

	extendCharMapWithDict(ct1Metrics.toDict());
	extendCharMapWithDict(dt2Metrics.toDict());

	return ProjectCharacteristics(instanceName, charMap);
}

template<class Func, class T>
void addStatisticalAggregates(map<string, float> &mapping, string prefix, Func f, vector<T> elems) {
	const auto add = [&](string name, float v) {
		mapping.insert(make_pair(prefix+"_"+name, v));
	};

	vector<float> mappedElems = Utils::constructVector<float>(elems.size(), [&](int ix) {
		return f(elems[ix]);
	});

	float min = *std::min_element(mappedElems.begin(), mappedElems.end());
	float max = *std::max_element(mappedElems.begin(), mappedElems.end());
	float mean = Utils::average(mappedElems);

	float variance = Utils::variance(mappedElems);
	float stddev = std::sqrt(variance);

	add("mean", mean);
	add("min", min);
	add("max", max);
	add("ratio", min / max);
	add("stddev", stddev);
	add("variance", variance);
}

std::map<std::string, float> ProjectWithOvertime::collectMesselisStats() const {
	vector<int> jobs = Utils::constructVector<int>(numJobs, [](int ix) { return ix; });
	vector<int> res = Utils::constructVector<int>(numRes, [](int ix) { return ix; });

	// size related features
	map<string, float> statMap = {
			{"total_capacity", Utils::sum(capacities)},
			{"res_job_ratio", (float)numRes / (float)numJobs},
			{"cap_job_ratio", (float)Utils::sum(capacities) / (float)numJobs},
	};

	addStatisticalAggregates(statMap, "cap_stats", [](int cap) {return cap;}, capacities);

	// resource constraint related features
	addStatisticalAggregates(statMap, "act_dem_res_count", [this](int j) {
		int numResReq = 0;
		eachResConst([&](int r) {
			numResReq += demands(j,r) > 0 ? 1 : 0;
		});
		return numResReq;
	}, jobs);

	addStatisticalAggregates(statMap, "act_dem_units", [this](int j) {
		int numUnitsReq = 0;
		eachResConst([&](int r) {
			numUnitsReq += demands(j,r);
		});
		return numUnitsReq;
	}, jobs);

	addStatisticalAggregates(statMap, "acts_per_res", [this](int r) {
		int numActReq = 0;
		eachJobConst([&](int j) {
			numActReq += demands(j,r) > 0 ? 1 : 0;
		});
		return numActReq;
	}, res);

	addStatisticalAggregates(statMap, "res_util_ratio", [this](int r) {
		int numUnitsReq = 0;
		eachJobConst([&](int j) {
			numUnitsReq += demands(j,r);
		});
		return (float)numUnitsReq / (float)capacities[r];
	}, res);

	// precedence constraint related features
	addStatisticalAggregates(statMap, "prec_per_act", [this](int job) {
		int precCount = 0;
		adjMx.foreach([&](int i, int j, char v) {
			precCount += v && (i == job || j  == job);
		});
		return precCount;
	}, jobs);

	addStatisticalAggregates(statMap, "predcount_per_act", [this](int job) {
		int predCount = 0;
		adjMx.foreach([&](int i, int j, char v) {
			predCount += v && (j == job);
		});
		return predCount;
	}, jobs);

	addStatisticalAggregates(statMap, "succcount_per_act", [this](int job) {
		int predCount = 0;
		adjMx.foreach([&](int i, int j, char v) {
			predCount += v && (i == job);
		});
		return predCount;
	}, jobs);

	// duration related features
	addStatisticalAggregates(statMap, "act_durations", [this](int j) {
		return durations[j];
	}, jobs);

	// FIXME: Implement n>1 times nested aggregations 6^n
	addStatisticalAggregates(statMap, "act_durations_for_res", [this](int r) {
		int totalDuration = 0;
		eachJobConst([&](int j) {
			totalDuration += demands(j,r) > 0 ? durations[j] : 0;
		});
		return totalDuration;
	}, res);

	return statMap;
}

std::string ProjectWithOvertime::plotAsAscii(const std::vector<int> &sts, int r) const {
	const auto jobNumToSingleChar = [this](int j) {
		if(j == 0) return '@';
		else if(j == lastJob) return '$';
		else if(j >= 1 && j <= 9) return static_cast<char>(j+'0');
		else if(j >= 10 && j <= 35) return static_cast<char>(j-10+'A');
		else if(j >= 36 && j <= 61) return static_cast<char>(j-36+'a');
		else return '0';
	};

	int height = capacities[r] + zmax[r];
	int width = sts[lastJob]+1;

	Matrix<char> mx(height, width, (int)' ');

	eachPeriodConst([&](int t) {
		vector<int> active;
		eachJobConst([&](int j) {
			if(isJobActiveInPeriod(sts, j, t)) {
				active.push_back(j);
			}
		});
		int ctr = height-1;
		for(int job : active) {
			for(int k=0; k<demands(job, r); k++)
				mx(ctr--, t) = jobNumToSingleChar(job);
		}
	});

	return mx.toStringCondensed();
}

std::map<std::string, float> ProjectWithOvertime::scheduleStatistics(const std::vector<int> &sts) const {
	return map<string, float> {
			{"makespan", makespan(sts)},
			{"totalCosts", totalCosts(sts)},
			{"revenue", revenue[makespan(sts)]},
			{"profit", calcProfit(sts)},
	};
}

void ProjectWithOvertime::printScheduleInformation(const std::vector<int> &sts) const {
	eachResConst([this, &sts](int r) {
		cout << "Schedule for resource " << r << endl;
		cout << plotAsAscii(sts, r) << endl;
	});
	for(const auto &pair : scheduleStatistics(sts)) {
		cout << pair.first << "=" << pair.second << endl;
	}
}

void ProjectWithOvertime::updateDerivedParameters() {
	Project::updateDerivedParameters();

	zmax.resize(numRes);
	zzero.resize(numRes);
	kappa.resize(numRes);
	revenue.resize(numPeriods);
	revenueExtrapolated.resize(numPeriods);

	/*eachRes([&](int r) {
		zmax[r] = capacities[r] / 2;
		kappa[r] = 0.5f;
	});*/

	computeRevenueFunction();
	computeExtrapolatedRevenueFunction();
}

std::pair<std::vector<std::string>, std::vector<float>> ProjectWithOvertime::flattenedRepresentation(const boost::optional<const ProjectCharacteristics&> chars) const {
	const list<string> includedChars = chars ? chars->getOrderedKeys() : list<string>{"nc", "avgBranch", "revWidth",
																					  "revSlope"};
	const unsigned int valueCount =
			numJobs + (chars ? includedChars.size() : numJobs * numJobs) + numJobs * numRes + numRes;
	vector<float> values(valueCount);
	vector<string> valueNames(valueCount);

	int ctr = 0;

	eachJobConst([&](int j) {
		valueNames[ctr] = "d_" + to_string(j);
		values[ctr++] = durations[j];
	});

	if (chars) {
		const auto c = *chars;
		for (const auto &k : includedChars) {
			valueNames[ctr] = k;
			values[ctr++] = c.getCharacteristic(k);
		}
	} else {
		eachJobPairConst([&](int i, int j) {
			valueNames[ctr] = "adj_" + to_string(i) + "_" + to_string(j);
			values[ctr++] = adjMx(i, j);
		});
	}

	eachJobResConst([&](int j, int r) {
		valueNames[ctr] = "k_" + to_string(j) + "_" + to_string(r);
		values[ctr++] = demands(j, r);
	});

	eachResConst([&](int r) {
		valueNames[ctr] = "K_" + to_string(r);
		values[ctr++] = capacities[r];
	});

	return make_pair(valueNames, values);
}

ProjectCharacteristics::ProjectCharacteristics(const std::string _instanceName,
											   const std::map<std::string, float> _characteristics) :
		instanceName(_instanceName),
		characteristics(_characteristics) {
	for(const auto &pair : characteristics) {
		orderedKeys.push_back(pair.first);
	}
	orderedKeys.sort();
}

std::string ProjectCharacteristics::csvHeaderLine() const {
	return "instance;" + boost::algorithm::join(orderedKeys, ";") + "\n";
}

std::string ProjectCharacteristics::toCsvLine() const {
	std::stringstream ss;
	ss << instanceName;
	for(const auto &key : orderedKeys) {
		float v = characteristics.at(key);
		/*if(std::isnan(v)) {
			printf("");
		}*/
		ss << ";" << characteristics.at(key);
	}
	ss << "\n";
	return ss.str();
}

float ProjectCharacteristics::getCharacteristic(std::string charName) const {
	return characteristics.at(charName);
}
