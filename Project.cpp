//
// Created by André Schnabel on 23.10.15.
//

#include <numeric>
#include <list>
#include <algorithm>
#include <string>
#include "Project.h"

Project::Project(string filename) {
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

    computeELSFTs();
}

template<class Func>
Matrix<int> Project::initResRem(Func code) const {
    Matrix<int> resRem(numRes, numPeriods);
    eachResPeriodConst([&](int r, int t) { resRem(r,t) = code(r, t); });
    return resRem;
}

vector<int> Project::serialSGS(const vector<int>& order) const {
    Matrix<int> resRem = initResRem([&](int r, int t) { return capacities[r]; });
	return serialSGSCore(order, resRem);
}

vector<int> Project::serialSGSForPartial(const vector<int> &sts, const vector<int> &order) const {
    Matrix<int> resRem = resRemForPartial(sts);

    vector<int> fts(numJobs), nsts;
    nsts = sts;
    eachJobConst([&](int j) {
        if(sts[j] != UNSCHEDULED) fts[j] = sts[j] + durations[j];
        else fts[j] = -1;
    });

    for (int job : order) {
        if(sts[job] == UNSCHEDULED) {
            int lastPredFinished = computeLastPredFinishingTimeForPartial(fts, job);
            int t;
            for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
            scheduleJobAt(job, t, nsts, fts, resRem);
        }
    }

    return nsts;
}

Matrix<int> Project::resRemForPartial(const vector<int> &sts) const {
    Matrix<int> resRem = initResRem([&](int r, int t) {
        int rem = capacities[r];
        eachJobConst([&](int j) {
            if(sts[j] != UNSCHEDULED && sts[j] < t && t <= sts[j])
                rem -= demands(j, r);
        });
        return rem;
    });
    return resRem;
}

pair<vector<int>, Matrix<int>> Project::serialSGS(const vector<int>& order, const vector<int>& z) const {
    Matrix<int> resRem = initResRem([&](int r, int t) { return capacities[r] + z[r]; });
	vector<int> sts = serialSGSCore(order, resRem);
	return make_pair(sts, resRem);
}

pair<vector<int>, Matrix<int>> Project::serialSGS(const vector<int>& order, const Matrix<int>& z) const {
    Matrix<int> resRem = initResRem([&](int r, int t) { return capacities[r] + z(r,t); });
	vector<int> sts = serialSGSCore(order, resRem);
	return make_pair(sts, resRem);
}

vector<int> Project::serialSGSCore(const vector<int>& order, Matrix<int>& resRem) const {
	vector<int> sts(numJobs), fts(numJobs);
	for (int job : order) {
		int lastPredFinished = computeLastPredFinishingTime(fts, job);
		int t;
		for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
		scheduleJobAt(job, t, sts, fts, resRem);
	}
	return sts;
}

void Project::parsePrecedenceRelation(const vector<string> &lines) {
    adjMx.resize(numJobs, numJobs);
    eachJob([&](int j) {
        auto nums = Utils::extractIntsFromLine(lines[18+j]);
        for(int i=3; i<nums.size(); i++)
            adjMx(j,nums[i]-1) = true;
    });
}

void Project::parseDurationsAndDemands(const vector<string> &lines) {
    durations.resize(numJobs);
    demands.resize(numJobs, numRes);
    eachJob([&](int j) {
        auto nums = Utils::extractIntsFromLine(lines[18+numJobs+4+ j]);
        durations[j] = nums[2];
        eachRes([&](int r) { demands(j,r) = nums[3+r]; });
    });
}

int Project::computeLastPredFinishingTime(const vector<int> &fts, int job) const {
	int lastPredFinished = 0;
    eachJobConst([&] (int j) { if (adjMx(j,job) && fts[j] > lastPredFinished) lastPredFinished = fts[j]; });
	return lastPredFinished;
}

int Project::computeLastPredFinishingTimeForPartial(const vector<int> &fts, int job) const {
    int lastPredFinished = 0;
    eachJobConst([&] (int j) { if (adjMx(j,job) && fts[j] != UNSCHEDULED && fts[j] > lastPredFinished) lastPredFinished = fts[j]; });
    return lastPredFinished;
}

int Project::computeFirstSuccStartingTime(const vector<int> &sts, int job) const {
    int firstSuccStarted = T;
    eachJobConst([&](int j) { if (adjMx(job,j) && sts[j] < firstSuccStarted) firstSuccStarted = sts[j]; });
    return firstSuccStarted;
}

bool Project::enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const {
    for(int tau = t + 1; tau <= t + durations[job]; tau++) {
        for(int r=0; r<numRes; r++)
            if(demands(job,r) > resRem(r,tau))
                return false;
    }
    return true;
}

void Project::scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, Matrix<int> &resRem) const {
	sts[job] = t;
	fts[job] = t + durations[job];
    eachResConst([&](int r) { for (int tau = t + 1; tau <= fts[job]; tau++) resRem(r,tau) -= demands(job,r); });
}

bool Project::jobBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
	for(int k = 0; k < curIndex; k++)
		if(order[k] == job)
			return true;
	return false;
}

bool Project::hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
    for(int i=0; i<numJobs; i++)
        if (adjMx(i,job) && !jobBeforeInOrder(i, curIndex, order)) return true;
	return false;
}

bool Project::jobAfterInOrder(int job, int curIndex, const vector<int>& order) const {
	for (int k = curIndex+1; k < order.size(); k++)
		if (order[k] == job)
			return true;
	return false;
}

bool Project::hasSuccNotAfterInOrder(int job, int curIndex, const vector<int>& order) const {
	for (int j = 0; j<numJobs; j++)
		if (adjMx(job, j) && !jobAfterInOrder(j, curIndex, order)) return true;
	return false;
}

bool Project::isOrderFeasible(const vector<int>& order) const {
	for (int i = 0; i < numJobs; i++) {
		if (hasPredNotBeforeInOrder(order[i], i, order))
			return false;
	}
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
		return !jobAfterInOrder(job, curIndex, order) && !hasSuccNotAfterInOrder(job, curIndex, order);
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

void Project::complementPartialWithSSGS(const vector<int> &order, int startIx, vector<int> &fts, Matrix<int> &resRem) const {
    for(int i=startIx; i<numJobs; i++) {
		int job = order[i];
		int lastPredFinished = computeLastPredFinishingTime(fts, job);
		int t;
		for (t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++);
		fts[job] = t + durations[job];
        
        eachResConst([&](int r) { for (int tau = t + 1; tau <= fts[job]; tau++) resRem(r, tau) -= demands(job, r); });
	}
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "InfiniteRecursion"
void Project::computeNodeDepths(int root, int curDepth, vector<int> &nodeDepths) {
    eachJob([&](int j) {
        if(adjMx(root, j) && (nodeDepths[j] == -1 || nodeDepths[j] > curDepth)) {
            nodeDepths[j] = curDepth;
            computeNodeDepths(j, curDepth+1, nodeDepths);
        }
    });
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
        eachJob([&](int j) { if(nodeDepths[j] == d) mapping[ctr++] = j; });
    }

    Matrix<char> newAdjMx(numJobs, numJobs);
    eachJobPair([&](int i, int j) {
        newAdjMx(i,j) = adjMx(mapping[i], mapping[j]);
    });

    Matrix<int> newDemands(numJobs, numRes);
    eachJobRes([&](int j, int r) {
        newDemands(j,r) = demands(mapping[j],mapping[r]);
    });

    vector<int> newDurations(numJobs);
        eachJob([&](int j) { newDurations[j] = durations[mapping[j]]; });

    durations = newDurations;
    adjMx = newAdjMx;
    demands = newDemands;
}

vector<int> Project::earliestStartSchedule(Matrix<int>& resRem) const {
	vector<int> ess(numJobs);
	for(int j : topOrder) {
		ess[j] = 0;
		for(int i = 0; i<numJobs; i++) {
			if(adjMx(i, j))
				ess[j] = Utils::max(ess[j], ess[i] + durations[i]);
		}
		eachResConst([&](int r) { for (int tau = ess[j] + 1; tau <= ess[j] + durations[j]; tau++) resRem(r, tau) -= demands(j, r); });
	}
	return ess;
}

