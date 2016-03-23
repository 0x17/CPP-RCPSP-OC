//
// Created by André Schnabel on 23.10.15.
//

#include "ProjectWithOvertime.h"
#include <numeric>
#include <list>
#include <algorithm>
#include <string>
#include "Project.h"

Project::Project(const string filename) : name(filename) {
    auto lines = Utils::readLines(filename);

    numJobs = Utils::extractIntFromStr(lines[5], "jobs \\(incl. supersource\\/sink \\):  (\\d+)");
	lastJob = numJobs - 1;
    numRes = Utils::extractIntFromStr(lines[8], "  - renewable                 :  (\\d+)   R");

    parsePrecedenceRelation(lines);
    parseDurationsAndDemands(lines);

    if(USE_DISPOSITION_METHOD)
        reorderDispositionMethod();

	T = accumulate(durations.begin(), durations.end(), 0);
    numPeriods = T+1;

    capacities = Utils::extractIntsFromLine(lines[18+numJobs*2+4+3]);

	topOrder = computeTopOrder();
	revTopOrder = computeReverseTopOrder();

    computeELSFTs();
}

vector<int> Project::serialSGS(const vector<int>& order) const {
    Matrix<int> resRem(numRes, numPeriods, [&](int r, int t) { return capacities[r]; });
	return serialSGSCore(order, resRem);
}

pair<vector<int>, Matrix<int>> Project::serialSGSForPartial(const vector<int>& sts, const vector<int>& order, Matrix<int>& resRem) const {
	vector<int> fts(numJobs, UNSCHEDULED), nsts = sts;
	transferAlreadyScheduledToFts(fts, sts);

	for(int job : order) {
		if (sts[job] == UNSCHEDULED) {
			int lastPredFinished = computeLastPredFinishingTimeForPartial(fts, job);
			int t;
			for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
			scheduleJobAt(job, t, nsts, fts, resRem);
		}
	}

	return make_pair(nsts, resRem);
}

pair<vector<int>, Matrix<int>> Project::serialSGSForPartial(const vector<int> &sts, const vector<int> &order) const {
    Matrix<int> resRem = resRemForPartial(sts);
	return serialSGSForPartial(sts, order, resRem);
}

Matrix<int> Project::resRemForPartial(const vector<int> &sts) const {
    Matrix<int> resRem(numRes, numPeriods, [&](int r, int t) {
        int rem = capacities[r];
        eachJobConst([&](int j) {
            if(sts[j] != UNSCHEDULED && sts[j] < t && t <= sts[j] + durations[j])
                rem -= demands(j, r);
        });
        return rem;
    });
    return resRem;
}

pair<vector<int>, Matrix<int>> Project::serialSGS(const vector<int>& order, const vector<int>& z, bool robust) const {
    Matrix<int> resRem(numRes, numPeriods, [&](int r, int t) { return capacities[r] + z[r]; });
	vector<int> sts = serialSGSCore(order, resRem, robust);
	eachResPeriodConst([&](int r, int t) { resRem(r, t) -= z[r]; });
	return make_pair(sts, resRem);
}

pair<vector<int>, Matrix<int>> Project::serialSGS(const vector<int>& order, const Matrix<int>& z, bool robust) const {
    Matrix<int> resRem(numRes, numPeriods, [&](int r, int t) { return capacities[r] + z(r,t); });
	vector<int> sts = serialSGSCore(order, resRem, robust);
	eachResPeriodConst([&](int r, int t) { resRem(r, t) -= z(r,t); });
	return make_pair(sts, resRem);
}

int Project::chooseEligibleWithLowestIndex(const vector<int>& sts, const vector<int>& order) const {
	for (int i = 0; i < numJobs; i++) {
		int j = order[i];
		if (sts[j] == UNSCHEDULED && allPredsScheduled(j, sts))
			return j;
	}
	throw runtime_error("No eligible job found!");
}

bool Project::allPredsScheduled(int j, const vector<int>& sts) const {
	for (int i = 0; i < numJobs; i++) {
		if (adjMx(i, j) && sts[i] == UNSCHEDULED)
			return false;
	}
	return true;
}

vector<int> Project::serialSGSCore(const vector<int>& order, Matrix<int>& resRem, bool robust) const {
	vector<int> sts(numJobs, UNSCHEDULED), fts(numJobs, UNSCHEDULED);
	for (int i = 0; i < numJobs; i++) {
		int job = robust ? chooseEligibleWithLowestIndex(sts, order) : order[i];
		int lastPredFinished = computeLastPredFinishingTime(fts, job);
		int t;
		for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
		scheduleJobAt(job, t, sts, fts, resRem);
	}
	return sts;
}

void Project::parsePrecedenceRelation(const vector<string> &lines) {
    adjMx.resize(numJobs, numJobs);
    EACH_JOB(
        auto nums = Utils::extractIntsFromLine(lines[18+j]);
        for(int i=3; i<nums.size(); i++)
            adjMx(j,nums[i]-1) = true;
    )
}

void Project::parseDurationsAndDemands(const vector<string> &lines) {
    durations.resize(numJobs);
    demands.resize(numJobs, numRes);
    EACH_JOB(
        auto nums = Utils::extractIntsFromLine(lines[18+numJobs+4+ j]);
        durations[j] = nums[2];
        EACH_RES(demands(j,r) = nums[3+r])
    )
}

int Project::computeLastPredFinishingTime(const vector<int> &fts, int job) const {
	int lastPredFinished = 0;
    EACH_JOB(if (adjMx(j,job) && fts[j] > lastPredFinished) lastPredFinished = fts[j])
	return lastPredFinished;
}

int Project::computeLastPredFinishingTimeForSts(const vector<int> &sts, int job) const {
    int lastPredFinished = 0;
    EACH_JOB(
        int ftj = sts[j] + durations[j];
        if (adjMx(j,job) && ftj > lastPredFinished) lastPredFinished = ftj)
    return lastPredFinished;
}

int Project::computeLastPredFinishingTimeForPartial(const vector<int> &fts, int job) const {
    int lastPredFinished = 0;
    EACH_JOB(if (adjMx(j,job) && fts[j] != UNSCHEDULED && fts[j] > lastPredFinished) lastPredFinished = fts[j])
    return lastPredFinished;
}

int Project::computeFirstSuccStartingTime(const vector<int> &sts, int job) const {
    int firstSuccStarted = T;
    EACH_JOB(if (adjMx(job,j) && sts[j] < firstSuccStarted) firstSuccStarted = sts[j])
    return firstSuccStarted;
}

bool Project::enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const {
    ACTIVE_PERIODS(job, t, EACH_RES(if(demands(job,r) > resRem(r,tau)) return false))
    return true;
}

void Project::scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, Matrix<int> &resRem) const {
	sts[job] = t;
	fts[job] = t + durations[job];
    EACH_RES(ACTIVE_PERIODS(job, t, resRem(r,tau) -= demands(job,r)))
}

void Project::scheduleJobAt(int job, int t, vector<int>& sts, Matrix<int>& resRem) const {
	sts[job] = t;
	EACH_RES(ACTIVE_PERIODS(job, t, resRem(r, tau) -= demands(job, r)))
}

bool Project::jobBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
	for(int k = 0; k < curIndex; k++)
		if(order[k] == job)
			return true;
	return false;
}

bool Project::hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
    EACH_JOBi(if (adjMx(i,job) && !jobBeforeInOrder(i, curIndex, order)) return true)
	return false;
}

bool Project::hasSuccNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
	EACH_JOB(if (adjMx(job, j) && !jobBeforeInOrder(j, curIndex, order)) return true)
	return false;
}

bool Project::isOrderFeasible(const vector<int>& order) const {
	EACH_JOBi(if(hasPredNotBeforeInOrder(order[i], i, order)) return false)
	return true;
}

template <class Pred>
vector<int> Project::topOrderComputationCore(Pred isEligible) const {
	vector<int> order(numJobs);
	
	for (int curIndex = 0; curIndex < numJobs; curIndex++) {
		for (int job = 0; job < numJobs; job++) {
			if (isEligible(curIndex, job, order)) {
				order[curIndex] = job;
				break;
			}
		}
	}

	return order;
}

vector<int> Project::computeTopOrder() const {
	return topOrderComputationCore([&](int curIndex, int job, const vector<int> &order) {
		return !jobBeforeInOrder(job, curIndex, order) && !hasPredNotBeforeInOrder(job, curIndex, order);
	});
}

vector<int> Project::computeReverseTopOrder() const {
	return topOrderComputationCore([&](int curIndex, int job, const vector<int> &order) {
		return !jobBeforeInOrder(job, curIndex, order) && !hasSuccNotBeforeInOrder(job, curIndex, order);
	});
}

void Project::computeELSFTs() {
    Utils::batchResize(numJobs, {&ests, &lsts, &efts, &lfts});

    for(int k=0; k<numJobs; k++) {
        int j = topOrder[k];
        ests[j] = computeLastPredFinishingTime(efts, j);
        efts[j] = ests[j] + durations[j];
    }

    for(int k=lastJob; k>=0; k--) {
        int j = topOrder[k];
        lfts[j] = computeFirstSuccStartingTime(lsts, j);
        lsts[j] = lfts[j] - durations[j];
    }
}

void Project::complementPartialWithSSGS(const vector<int> &order, int startIx, vector<int> &fts, Matrix<int> &resRem, bool robust) const {
    for(int i=startIx; i<numJobs; i++) {
		int job = robust ? chooseEligibleWithLowestIndex(fts, order) : order[i];
		int lastPredFinished = computeLastPredFinishingTime(fts, job);
		int t;
		for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
		fts[job] = t + durations[job];
        EACH_RES(ACTIVE_PERIODS(job, t, resRem(r, tau) -= demands(job, r)))
	}
}

Matrix<int> Project::normalCapacityProfile() const {
	Matrix<int> resRem(numRes, numPeriods, [this](int r, int t) { return capacities[r]; });
	return resRem;
}

vector<int> Project::emptySchedule() const {
	vector<int> sts(numJobs, UNSCHEDULED);
	return sts;
}

#pragma warning (disable : 4068) 
#pragma clang diagnostic push
#pragma ide diagnostic ignored "InfiniteRecursion"
void Project::computeNodeDepths(int root, int curDepth, vector<int> &nodeDepths) {
    EACH_JOB(
        if(adjMx(root, j) && (nodeDepths[j] == -1 || nodeDepths[j] > curDepth)) {
            nodeDepths[j] = curDepth;
            computeNodeDepths(j, curDepth+1, nodeDepths);
        }
    )
}
#pragma clang diagnostic pop

void Project::reorderDispositionMethod() {
    vector<int> nodeDepths(numJobs, -1);
    nodeDepths[0] = 0;
    computeNodeDepths(0, 1, nodeDepths);

    int maxDepth = *max_element(nodeDepths.begin(), nodeDepths.end());

    vector<int> mapping(numJobs);
    int ctr = 0;
    for(int d = 0; d <= maxDepth; d++) {
        EACH_JOB(if(nodeDepths[j] == d) mapping[ctr++] = j)
    }

    Matrix<char> newAdjMx(numJobs, numJobs);
    EACH_JOB_PAIR(newAdjMx(i,j) = adjMx(mapping[i], mapping[j]))

    Matrix<int> newDemands(numJobs, numRes);
    EACH_JOB_RES(newDemands(j,r) = demands(mapping[j],mapping[r]))

    vector<int> newDurations(numJobs);
    EACH_JOB(newDurations[j] = durations[mapping[j]])

    durations = newDurations;
    adjMx = newAdjMx;
    demands = newDemands;
}

vector<int> Project::earliestStartSchedule(Matrix<int>& resRem) const {
	vector<int> ess(numJobs);
	for(int j : topOrder) {
		ess[j] = 0;
        EACH_JOBi(if(adjMx(i, j)) ess[j] = Utils::max(ess[j], ess[i] + durations[i]))
		EACH_RES(ACTIVE_PERIODS(j, ess[j], resRem(r, tau) -= demands(j, r)))
	}
	return ess;
}

int Project::latestStartingTimeInPartial(const vector<int>& sts) const {
    int maxSt = 0;
    EACH_JOBi(if(sts[i] != UNSCHEDULED && sts[i] > maxSt) maxSt = sts[i])
    return maxSt;
}

int Project::earliestStartingTimeInPartial(const vector<int> &sts) const {
    int minSt = numeric_limits<int>::max();
    EACH_JOB(if(sts[j] != UNSCHEDULED && sts[j] < minSt) minSt = sts[j])
    return minSt;
}

vector<int> Project::earliestStartingTimesForPartial(const vector<int>& sts) const {
	vector<int> ests(numJobs, UNSCHEDULED);

	transferAlreadyScheduled(ests, sts);

	for (int j : topOrder) {
        if(sts[j] != UNSCHEDULED) continue;
		ests[j] = 0;
        EACH_JOBi(if(adjMx(i, j)) ests[j] = Utils::max(ests[j], ests[i] + durations[i]))
	}

	return ests;
}

vector<int> Project::latestFinishingTimesForPartial(const vector<int>& sts, int deadline) const {
	vector<int> lfts(numJobs, UNSCHEDULED);

    transferAlreadyScheduledToFts(lfts, sts);

	for (int i : revTopOrder) {
        if (sts[i] != UNSCHEDULED) continue;
		lfts[i] = deadline;
		EACH_JOB(if(adjMx(i, j)) lfts[i] = Utils::min(lfts[i], lfts[j] - durations[j]))
	}

	return lfts;
}

void Project::transferAlreadyScheduled(vector<int> &destSts, const vector<int> &partialSts) const {
    EACH_JOBi(if(partialSts[i] != UNSCHEDULED) destSts[i] = partialSts[i])
}

void Project::transferAlreadyScheduledToFts(vector<int> &destFts, const vector<int> &partialSts) const {
	EACH_JOBi(if (partialSts[i] != UNSCHEDULED) destFts[i] = partialSts[i] + durations[i])
}