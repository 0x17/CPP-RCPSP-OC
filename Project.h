//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECT_H
#define SSGS_PROJECT_H

#include <map>
#include "Utils.h"

// Set range iteration macros
#define EACH_COMMON(ctr, upperbound, code) for(int ctr=0; ctr<upperbound; ctr++) { code; }
#define EACH_RES(code) EACH_COMMON(r, numRes, code)
#define EACH_JOB(code) EACH_COMMON(j, numJobs, code)
#define EACH_JOBi(code) EACH_COMMON(i, numJobs, code)
#define EACH_PERIOD(code) EACH_COMMON(t, numPeriods, code)

class Project {
public:
    int numJobs, numRes, numPeriods;
    vector<vector<bool>> adjMx;
    vector<int> durations, capacities;
    vector<vector<int>> demands;

    vector<int> topOrder;

    Project(string filename);
    virtual ~Project() {}

    vector<int> serialSGS(const vector<int> order, const vector<int> zr) const;

private:
    void parsePrecedenceRelation(const vector<string> &lines);
    void parseDurationsAndDemands(const vector<string> &lines);

    bool enoughCapacityForJob(int job, int t, vector<vector<int>> resRem) const;
    int computeLastPredFinishingTime(const vector<int> &fts, int job) const;
    void scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, vector<vector<int>> &resRem) const;
};

#endif //SSGS_PROJECT_H
