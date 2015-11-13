//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECT_H
#define SSGS_PROJECT_H

#include "Utils.h"
#include "Matrix.h"

#define EACH_FUNC(name, constname, it, ub) \
    template<class Func> \
    void name(Func code) { for(int it=0; it<ub; it++) { code(it); }} \
    template<class Func> \
    void constname(Func code) const { for(int it=0; it<ub; it++) { code(it); }}

#define EACH_FUNC_PAIR(name, constname, it1, it2, ub1, ub2) \
    template<class Func> \
    void name(Func code) { for(int it1=0; it1<ub1; it1++) { for(int it2=0; it2<ub2; it2++) { code(it1, it2); } }} \
    template<class Func> \
    void constname(Func code) const { for(int it1=0; it1<ub1; it1++) { for(int it2=0; it2<ub2; it2++) { code(it1, it2); } }}

typedef pair<vector<int>, Matrix<int>> SGSResult;

class Project {
public:
    int numJobs, numRes, numPeriods, T, lastJob;
    Matrix<char> adjMx;
    vector<int> durations, capacities;
    Matrix<int> demands;

	vector<int> topOrder, revTopOrder;

    vector<int> ests, lsts, efts, lfts;

    const bool USE_DISPOSITION_METHOD = false;

    enum { UNSCHEDULED = -1 };

	explicit Project(string filename);
    virtual ~Project() {}

	vector<int> serialSGS(const vector<int>& order) const;
	pair<vector<int>, Matrix<int>> serialSGSForPartial(const vector<int> &sts, const vector<int> &order, Matrix<int> &resRem) const;
    pair<vector<int>, Matrix<int>> serialSGSForPartial(const vector<int> &sts, const vector<int> &order) const;
	SGSResult serialSGS(const vector<int>& order, const vector<int>& zr) const;
	SGSResult serialSGS(const vector<int>& order, const Matrix<int>& zrt) const;

	bool jobBeforeInOrder(int job, int curIndex, const vector<int>& order) const;
	bool hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const;

	bool jobAfterInOrder(int job, int curIndex, const vector<int>& order) const;
	bool hasSuccNotAfterInOrder(int job, int curIndex, const vector<int>& order) const;

	bool isOrderFeasible(const vector<int> &order) const;

    int makespan(const vector<int>& sts) const;

    EACH_FUNC(eachJob, eachJobConst, j, numJobs)
    EACH_FUNC(eachJobi, eachJobiConst, i, numJobs)
    EACH_FUNC(eachRes, eachResConst, r, numRes)
    EACH_FUNC(eachPeriod, eachPeriodConst, t, numPeriods)

    EACH_FUNC_PAIR(eachJobPair, eachJobPairConst, i, j, numJobs, numJobs)
    EACH_FUNC_PAIR(eachResPeriod, eachResPeriodConst, r, t, numRes, numPeriods)
    EACH_FUNC_PAIR(eachJobRes, eachJobResConst, j, r, numJobs, numRes)

    template<class Func>
    void timeWindow(int j, Func code) const;

	Matrix<int> resRemForPartial(const vector<int> &sts) const;

    int computeLastPredFinishingTime(const vector<int> &fts, int job) const;
    int computeLastPredFinishingTimeForSts(const vector<int> &sts, int job) const;
    int latestStartingTimeInPartial(const vector<int> &sts) const;
    int earliestStartingTimeInPartial(const vector<int> &sts) const;

    vector<int> earliestStartingTimesForPartial(const vector<int> &sts) const;
    vector<int> latestFinishingTimesForPartial(const vector<int> &sts) const;

protected:
    void complementPartialWithSSGS(const vector<int>& order, int startIx, vector<int> &fts, Matrix<int> &resRem) const;

	vector<int> serialSGSCore(const vector<int>& order, Matrix<int> &resRem) const;

    bool enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const;

    int computeFirstSuccStartingTime(const vector<int> &sts, int job) const;
    int computeLastPredFinishingTimeForPartial(const vector<int> &fts, int job) const;
    void scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, Matrix<int> &resRem) const;

	vector<int> earliestStartSchedule(Matrix<int> & resRem) const;

    void transferAlreadyScheduled(vector<int> &destSts, const vector<int> &partialSts) const;

private:
    void parsePrecedenceRelation(const vector<string> &lines);
    void parseDurationsAndDemands(const vector<string> &lines);

    void reorderDispositionMethod();

	template <class Pred>
	vector<int> topOrderComputationCore(Pred isEligible) const;
	vector<int> computeTopOrder() const;
	vector<int> computeReverseTopOrder() const;

    void computeELSFTs();

    void computeNodeDepths(int root, int curDepth, vector<int> &nodeDepths);
};

template<class Func>
inline void Project::timeWindow(int j, Func code) const {
    for(int t=efts[j]; t<=lfts[j]; t++) { code(t); }
}

inline int Project::makespan(const vector<int>& sts) const {
    return sts[lastJob];
}

#endif //SSGS_PROJECT_H
