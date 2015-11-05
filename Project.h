//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECT_H
#define SSGS_PROJECT_H

#include <map>
#include "Utils.h"

// Set range iteration macros
#define EACH_RNG(ctr, upperbound, code) for(int ctr=0; ctr<upperbound; ctr++) { code; }

#define EACH_RES(code) EACH_RNG(r, numRes, code)
#define EACH_JOB(code) EACH_RNG(j, numJobs, code)
#define EACH_JOBi(code) EACH_RNG(i, numJobs, code)
#define EACH_PERIOD(code) EACH_RNG(t, numPeriods, code)

#define P_EACH_JOB(code) EACH_RNG(j, p.numJobs, code)
#define P_EACH_JOBi(code) EACH_RNG(i, p.numJobs, code)
#define P_EACH_RES(code) EACH_RNG(r, p.numRes, code)
#define P_EACH_PERIOD(code) EACH_RNG(t, p.numPeriods, code)

typedef pair<vector<int>, Matrix<int>> SGSResult;

class Project {
public:
    int numJobs, numRes, numPeriods, T;
    Matrix<char> adjMx;
    vector<int> durations, capacities;
    Matrix<int> demands;

    vector<int> topOrder;

    vector<int> ests, lsts, efts, lfts;

	explicit Project(string filename);
    virtual ~Project() {}

	vector<int> serialSGS(const vector<int>& order) const;
	SGSResult serialSGS(const vector<int>& order, const vector<int>& zr) const;
	SGSResult serialSGS(const vector<int>& order, const Matrix<int>& zrt) const;

	bool jobBeforeInOrder(int job, int curIndex, const vector<int>& order) const;
	bool hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const;

	bool isOrderFeasible(const vector<int> &order) const;

protected:
    void complementPartialWithSSGS(const vector<int>& order, int startIx, vector<int> &fts, Matrix<int> &resRem) const;

	vector<int> serialSGSCore(const vector<int>& order, Matrix<int> &resRem) const;

    bool enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const;
    int computeLastPredFinishingTime(const vector<int> &fts, int job) const;
    int computeFirstSuccStartingTime(const vector<int> &sts, int job) const;
    void scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, Matrix<int> &resRem) const;

private:
    void parsePrecedenceRelation(const vector<string> &lines);
    void parseDurationsAndDemands(const vector<string> &lines);

	vector<int> computeTopOrder() const;
    void computeELSFTs();
};

#endif //SSGS_PROJECT_H
