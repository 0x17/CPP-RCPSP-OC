//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECT_H
#define SSGS_PROJECT_H

#include "Utils.h"
#include "Matrix.h"

#define EACH_COMMON(ix, ubExcl, code) \
    for(int ix=0; ix<ubExcl; ix++) {\
        code; \
    }

#define EACH_COMMON_2D(ix1, ix2, ub1Excl, ub2Excl, code) \
    for(int ix1=0; ix1<ub1Excl; ix1++) \
        for(int ix2=0; ix2<ub2Excl; ix2++) {\
            code; \
        }

#define EACH_JOB(code) EACH_COMMON(j, numJobs, code)
#define EACH_JOBi(code) EACH_COMMON(i, numJobs, code)
#define EACH_RES(code) EACH_COMMON(r, numRes, code)
#define EACH_PERIOD(code) EACH_COMMON(t, numPeriods, code)

#define EACH_RES_PERIOD(code) EACH_COMMON_2D(r, t, numRes, numPeriods, code)
#define EACH_JOB_RES(code) EACH_COMMON_2D(j, r, numJobs, numRes, code)
#define EACH_JOB_PAIR(code) EACH_COMMON_2D(i, j, numJobs, numJobs, code)

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

#define ACTIVE_PERIODS(j, stj, code) \
    for(int tau = stj + 1; tau <= stj + durations[j]; tau++) { \
        code; \
    }

//typedef pair<vector<int>, Matrix<int>> SGSResult;
struct SGSResult {
	vector<int> sts;
	Matrix<int> resRem;

	SGSResult(vector<int> _sts, Matrix<int> _resRem) : sts(_sts), resRem(_resRem) {}
	SGSResult() {}
};

class Project {
public:
	const string name, instanceName;

    int numJobs, numRes, numPeriods, T, lastJob;
    Matrix<char> adjMx;
    vector<int> durations, capacities;
    Matrix<int> demands;

	vector<int> topOrder, revTopOrder;

    vector<int> ests, lsts, efts, lfts;

    const bool USE_DISPOSITION_METHOD = false;

	enum { UNSCHEDULED = -1 };

	explicit Project(const string filename);
    virtual ~Project() {}

	vector<int> serialSGS(const vector<int>& order) const;
	pair<vector<int>, Matrix<int>> serialSGSForPartial(const vector<int> &sts, const vector<int> &order, Matrix<int> &resRem) const;
    pair<vector<int>, Matrix<int>> serialSGSForPartial(const vector<int> &sts, const vector<int> &order) const;
	SGSResult serialSGS(const vector<int>& order, const vector<int>& zr, bool robust = false) const;
	SGSResult serialSGS(const vector<int>& order, const Matrix<int>& zrt, bool robust = false) const;

	static bool jobBeforeInOrder(int job, int curIndex, const vector<int>& order);
	bool hasPredNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const;
	bool hasSuccNotBeforeInOrder(int job, int curIndex, const vector<int>& order) const;

	bool isOrderFeasible(const vector<int> &order) const;

    int makespan(const vector<int>& sts) const;
	int makespan(const SGSResult& res) const;

	EACH_FUNC(eachJob, eachJobConst, j, numJobs)
	EACH_FUNC(eachJobi, eachJobiConst, i, numJobs)
	EACH_FUNC(eachRes, eachResConst, r, numRes)
	EACH_FUNC(eachPeriod, eachPeriodConst, t, numPeriods)
	EACH_FUNC(eachPeriodBounded, eachPeriodBoundedConst, t, heuristicMaxMs);

	EACH_FUNC_PAIR(eachJobPair, eachJobPairConst, i, j, numJobs, numJobs)
	EACH_FUNC_PAIR(eachResPeriod, eachResPeriodConst, r, t, numRes, numPeriods)
	EACH_FUNC_PAIR(eachResPeriodBounded, eachResPeriodBoundedConst, r, t, numRes, heuristicMaxMs)
    EACH_FUNC_PAIR(eachJobRes, eachJobResConst, j, r, numJobs, numRes)

    template<class Func>
    void timeWindow(int j, Func code) const;

	template<class Func>
	void timeWindowBounded(int j, Func code) const;

	template<class Func>
	void eachJobTimeWindow(Func code) const;

	template<class Func>
	void eachJobTimeWindowBounded(Func code) const;

	template <class Func>
	void demandInPeriodMIP(int j, int t, Func code) const;

	Matrix<int> resRemForPartial(const vector<int> &sts) const;

    int computeLastPredFinishingTime(const vector<int> &fts, int job) const;
    int computeLastPredFinishingTimeForSts(const vector<int> &sts, int job) const;
    int latestStartingTimeInPartial(const vector<int> &sts) const;
    int earliestStartingTimeInPartial(const vector<int> &sts) const;

    vector<int> earliestStartingTimesForPartial(const vector<int> &sts) const;
    vector<int> latestFinishingTimesForPartial(const vector<int> &sts, int deadline) const;

	int chooseEligibleWithLowestIndex(const vector<int> &sts, const vector<int> &order) const;
	int chooseEligibleWithLowestIndex(const vector<bool> &unscheduled, const vector<int> &order) const;
	int chooseEligibleWithHighestIndex(const vector<bool>& unscheduled, const vector<int>& order) const;


	void complementPartialWithSSGS(const vector<int>& order, int startIx, vector<int> &fts, Matrix<int> &resRem, bool robust = false) const;

	Matrix<int> normalCapacityProfile() const;

	vector<int> emptySchedule() const;

	int getHeuristicMaxMakespan() const;

	bool isScheduleFeasible(const vector<int>& sts) const;
	bool isSchedulePrecedenceFeasible(const vector<int>& sts) const;
	virtual bool isScheduleResourceFeasible(const vector<int>& sts) const;
	bool isScheduleResourceFeasible(const vector<int>& sts, const vector<int>& zr) const;

protected:
	bool allPredsScheduled(int j, const vector<int> &sts) const;
	bool allPredsScheduled(int j, const vector<bool>& unscheduled) const;
	bool allSuccsScheduled(int j, const vector<bool>& unscheduled) const;

	vector<int> serialSGSCore(const vector<int>& order, Matrix<int> &resRem, bool robust = false) const;

    bool enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const;

    int computeFirstSuccStartingTime(const vector<int> &sts, int job) const;
	int computeFirstSuccStartingTimeForPartial(const vector<int> &fts, int job) const;
    int computeLastPredFinishingTimeForPartial(const vector<int> &fts, int job) const;
    void scheduleJobAt(int job, int t, vector<int> &sts, vector<int> &fts, Matrix<int> &resRem) const;
	void scheduleJobAt(int job, int t, vector<int> &sts, Matrix<int> &resRem) const;

	vector<int> earliestStartSchedule(Matrix<int> & resRem) const;

    void transferAlreadyScheduled(vector<int> &destSts, const vector<int> &partialSts) const;
	void transferAlreadyScheduledToFts(vector<int> &destFts, const vector<int> &partialSts) const;

	void shiftScheduleLeftBy(int offset, vector<int> &sts, Matrix<int> &resRem) const;

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

	int heuristicMaxMs;
};

template<class Func>
inline void Project::timeWindow(int j, Func code) const {
    for(int t=efts[j]; t<=lfts[j]; t++) { code(t); }
}

template<class Func>
inline void Project::timeWindowBounded(int j, Func code) const {
	for (int t = efts[j]; t <= min(lfts[j], heuristicMaxMs); t++) { code(t); }
}

template <class Func>
inline void Project::eachJobTimeWindow(Func code) const {
	eachJobConst([&](int j) {
		for(int t = efts[j]; t <= lfts[j]; t++) {
			code(j, t);
		}
	});
}

template <class Func>
inline void Project::eachJobTimeWindowBounded(Func code) const {
	eachJobConst([&](int j) {
		for (int t = efts[j]; t <= min(lfts[j], heuristicMaxMs); t++) {
			code(j, t);
		}
	});
}

template <class Func>
inline void Project::demandInPeriodMIP(int j, int t, Func code) const {
	for(int tau = t; tau < min(t + durations[j], getHeuristicMaxMakespan() + 1); tau++) {
		code(tau);
	}
}

inline int Project::makespan(const vector<int>& sts) const {
    return sts[lastJob];
}

inline int Project::getHeuristicMaxMakespan() const {
	return heuristicMaxMs;
}

#endif //SSGS_PROJECT_H
