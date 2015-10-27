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

    numPeriods = accumulate(durations.begin(), durations.end(), 0);

    capacities = Utils::extractIntsFromLine(lines[18+numJobs*2+4+3]);

	topOrder = computeTopOrder();
}

void Project::parsePrecedenceRelation(const vector<string> &lines) {
    Utils::resizeMatrix(adjMx, numJobs, numJobs);

    EACH_JOB(
        auto nums = Utils::extractIntsFromLine(lines[18+j]);
        for(int i=3; i<nums.size(); i++) {
            adjMx[j][nums[i]-1] = true;
        }
    )
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

vector<int> Project::serialSGS(const vector<int> & order, const vector<int> & zr) const {
    vector<int> sts(numJobs), fts(numJobs);

    vector<vector<int>> resRem = Utils::initMatrix<int>(numRes, numPeriods);
    EACH_RES(EACH_PERIOD(resRem[r][t] = capacities[r] + zr[r]))

    for(int job : order) {
        int lastPredFinished = computeLastPredFinishingTime(fts, job);
        int t;
        for(t = lastPredFinished; !enoughCapacityForJob(job, t, resRem); t++) ;
        scheduleJobAt(job, t, sts, fts, resRem);
    }
    return sts;
}

int Project::computeLastPredFinishingTime(const vector<int> &fts, int job) const {
	int lastPredFinished = 0;
	EACH_JOB(if (adjMx[j][job] && fts[j] > lastPredFinished) lastPredFinished = fts[j])
	return lastPredFinished;
}

bool Project::enoughCapacityForJob(int job, int t, vector<vector<int>> & resRem) const {
    for(int tau = t + 1; tau <= t + durations[job]; tau++) {
        EACH_RES(if(demands[job][r] > resRem[r][tau]) return false)
    }
    return true;
}

inline void Project::scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, vector<vector<int>> &resRem) const {
	sts[job] = t;
	fts[job] = t + durations[job];
	EACH_RES(for (int tau = t + 1; tau <= fts[job]; tau++) resRem[r][t] -= demands[job][r];)
}

vector<int> Project::computeTopOrder() {
	vector<int> order;
	// TODO: Implement me!
	return order;
}

