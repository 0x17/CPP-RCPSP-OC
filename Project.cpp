//
// Created by André Schnabel on 23.10.15.
//

#include <numeric>
#include "Project.h"

Project::Project(string filename) {
    auto lines = Utils::readLines(filename);

    numJobs = Utils::extractIntFromStr(lines[5], "jobs \\(incl. supersource\\/sink \\):  (\\d+)");
    numRes = Utils::extractIntFromStr(lines[8], "  - renewable                 :  (\\d+)   R");

    parsePrecedenceRelation(lines);
    parseDurationsAndDemands(lines);

	T = accumulate(durations.begin(), durations.end(), 0);
    numPeriods = T+1;

    capacities = Utils::extractIntsFromLine(lines[18+numJobs*2+4+3]);

	topOrder = computeTopOrder();

    computeELSFTs();
}

#define INIT_RES_REM(code) \
	vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods); \
	EACH_RES(EACH_PERIOD(code));

vector<int> Project::serialSGS(const vector<int>& order) const {
	INIT_RES_REM(resRem[r][t] = capacities[r])
	return serialSGSCore(order, resRem);
}

pair<vector<int>, vector<vector<int>>> Project::serialSGS(const vector<int>& order, const vector<int>& z) const {
	INIT_RES_REM(resRem[r][t] = capacities[r] + z[r])
	vector<int> sts = serialSGSCore(order, resRem);
	return make_pair(sts, resRem);
}

pair<vector<int>, vector<vector<int>>> Project::serialSGS(const vector<int>& order, const vector<vector<int>>& z) const {
	INIT_RES_REM(resRem[r][t] = capacities[r] + z[r][t])
	vector<int> sts = serialSGSCore(order, resRem);
	return make_pair(sts, resRem);
}

vector<int> Project::serialSGSCore(const vector<int>& order, vector<vector<int>>& resRem) const {
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
    Utils::resizeMatrix(adjMx, numJobs, numJobs);

    EACH_JOB(
        auto nums = Utils::extractIntsFromLine(lines[18+j]);
        for(int i=3; i<nums.size(); i++)
            adjMx[j][nums[i]-1] = true)
}

void Project::parseDurationsAndDemands(const vector<string> &lines) {
    durations.resize(numJobs);
    Utils::resizeMatrix(demands, numJobs, numRes);

    EACH_JOB(
        auto nums = Utils::extractIntsFromLine(lines[18+numJobs+4+ j]);
        durations[j] = nums[2];
        EACH_RES(demands[j][r] = nums[3+r])
    )
}

int Project::computeLastPredFinishingTime(const vector<int> &fts, int job) const {
	int lastPredFinished = 0;
	EACH_JOB(if (adjMx[j][job] && fts[j] > lastPredFinished) lastPredFinished = fts[j])
	return lastPredFinished;
}

int Project::computeFirstSuccStartingTime(const vector<int> &sts, int job) const {
    int firstSuccStarted = T;
    EACH_JOB(if (adjMx[job][j] && sts[j] < firstSuccStarted) firstSuccStarted = sts[j])
    return firstSuccStarted;
}

bool Project::enoughCapacityForJob(int job, int t, vector<vector<int>> & resRem) const {
    for(int tau = t + 1; tau <= t + durations[job]; tau++) {
        EACH_RES(if(demands[job][r] > resRem[r][tau]) return false)
    }
    return true;
}

void Project::scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, vector<vector<int>> &resRem) const {
	sts[job] = t;
	fts[job] = t + durations[job];
	EACH_RES(for (int tau = t + 1; tau <= fts[job]; tau++) resRem[r][tau] -= demands[job][r];)
}

bool Project::jobBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
	for(int k = 0; k < curIndex; k++)
		if(order[k] == job)
			return true;
	return false;
}

bool Project::hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const {
	EACH_JOBi(if (adjMx[i][job] && !jobBeforeInOrder(i, curIndex, order)) return true)
	return false;
}

bool Project::isOrderFeasible(const vector<int>& order) const {
	for (int i = 0; i < numJobs; i++) {
		if (hasPredNotBeforeInOrder(order[i], i, order))
			return false;
	}
	return true;
}

vector<int> Project::computeTopOrder() const {
	vector<int> order(numJobs);
	for (int curIndex = 0; curIndex < numJobs; curIndex++) {
		for(int job = 0; job < numJobs; job++) {
			if(!jobBeforeInOrder(job, curIndex, order) && !hasPredNotBeforeInOrder(job, curIndex, order)) {
				order[curIndex] = job;
				break;
			}
		}
	}
	return order;
}

void Project::computeELSFTs() {
    Utils::batchResize(numJobs, {&ests, &lsts, &efts, &lfts});

    for(int k=0; k<numJobs; k++) {
        int j = topOrder[k];
        ests[j] = computeLastPredFinishingTime(efts, j);
        efts[j] = ests[j] + durations[j];
    }

    for(int k=numJobs-1; k>=0; k--) {
        int j = topOrder[k];
        lfts[j] = computeFirstSuccStartingTime(lsts, j);
        lsts[j] = lfts[j] - durations[j];
    }
}

void Project::complementPartialWithSSGS(const vector<int> &order, int startIx, vector<int> &fts, vector<vector<int>> &resRem) const {
    for(int i=startIx; i<numJobs; i++) {
        
    }

}
