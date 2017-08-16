//
// Created by André Schnabel on 23.10.15.
//

#pragma once

#include "Utils.h"
#include "Matrix.h"

#include <boost/filesystem/path.hpp>

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

//typedef pair<std::vector<int>, Matrix<int>> SGSResult;
struct SGSResult {
	std::vector<int> sts;
	Matrix<int> resRem;
	int numSchedulesGenerated = 1;

	SGSResult(std::vector<int> _sts, Matrix<int> _resRem, int _numSchedulesGenerated = 1) : sts(_sts), resRem(_resRem), numSchedulesGenerated(_numSchedulesGenerated) {}
	SGSResult() {}
};

class Project {
public:
	std::string name, instanceName;

    int numJobs, numRes, numPeriods, T, lastJob;
    Matrix<char> adjMx;
	std::vector<int> durations, capacities;
    Matrix<int> demands;

	std::vector<int> topOrder, revTopOrder;

	std::vector<int> ests, lsts, efts, lfts;

    const bool USE_DISPOSITION_METHOD = false;

	enum { UNSCHEDULED = -1 };

	explicit Project(const std::string &filename);
	Project(const std::string& projectName, const std::string& s);
	virtual ~Project() {}

	std::vector<int> serialSGS(const std::vector<int>& order) const;
	std::pair<std::vector<int>, Matrix<int>> serialSGSForPartial(const std::vector<int> &sts, const std::vector<int> &order, Matrix<int> &resRem) const;
	std::pair<std::vector<int>, Matrix<int>> serialSGSForPartial(const std::vector<int> &sts, const std::vector<int> &order) const;
	SGSResult serialSGS(const std::vector<int>& order, const std::vector<int>& zr, bool robust = false) const;
	SGSResult serialSGS(const std::vector<int>& order, const Matrix<int>& zrt, bool robust = false) const;
	SGSResult serialSGSWithRandomKey(const std::vector<float> &rk) const;

	static bool jobBeforeInOrder(int job, int curIndex, const std::vector<int>& order);
	bool hasPredNotBeforeInOrder(int job, int curIndex, const std::vector<int>& order) const;
	bool hasSuccNotBeforeInOrder(int job, int curIndex, const std::vector<int>& order) const;

	bool isOrderFeasible(const std::vector<int> &order) const;

    int makespan(const std::vector<int>& sts) const;
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

	Matrix<int> resRemForPartial(const std::vector<int> &sts) const;

    int computeLastPredFinishingTime(const std::vector<int> &fts, int job) const;
    int computeLastPredFinishingTimeForSts(const std::vector<int> &sts, int job) const;
    int latestStartingTimeInPartial(const std::vector<int> &sts) const;
    int earliestStartingTimeInPartial(const std::vector<int> &sts) const;

	std::vector<int> earliestStartingTimesForPartial(const std::vector<int> &sts) const;
	std::vector<int> latestFinishingTimesForPartial(const std::vector<int> &sts, int deadline) const;

	int chooseEligibleWithLowestIndex(const std::vector<int> &sts, const std::vector<int> &order) const;
	int chooseEligibleWithLowestIndex(const std::vector<bool> &unscheduled, const std::vector<int> &order) const;
	int chooseEligibleWithHighestIndex(const std::vector<bool>& unscheduled, const std::vector<int>& order) const;
	int chooseEligibleWithHighestPriority(const std::vector<int> &sts, const std::vector<float> &rk) const;
	
	void complementPartialWithSSGS(const std::vector<int>& order, int startIx, std::vector<int> &fts, Matrix<int> &resRem, bool robust = false) const;

	Matrix<int> normalCapacityProfile() const;

	std::vector<int> emptySchedule() const;

	int getHeuristicMaxMakespan() const;

	bool isScheduleFeasible(const std::vector<int>& sts) const;
	bool isSchedulePrecedenceFeasible(const std::vector<int>& sts) const;
	virtual bool isScheduleResourceFeasible(const std::vector<int>& sts) const;
	bool isScheduleResourceFeasible(const std::vector<int>& sts, const std::vector<int>& zr) const;

	bool isResRemValid(const std::vector<int>& sts, const Matrix<int>& resRem) const;

	static std::string coreInstanceName(const std::string & parentPath, const std::string & filename);

	std::vector<int> standardizeRandomKey(const std::vector<float> &rk) const;
	std::vector<int> scheduleToActivityList(const std::vector<int> &sts) const;
	std::vector<int> activityListToRankVector(const std::vector<int> &order) const;

	int earliestJobInScheduleNotAlreadyTaken(const std::vector<int> &sts, const std::vector<bool> &alreadyTaken) const;

protected:
	bool allPredsScheduled(int j, const std::vector<int> &sts) const;
	bool allPredsScheduled(int j, const std::vector<bool>& unscheduled) const;
	bool allSuccsScheduled(int j, const std::vector<bool>& unscheduled) const;

	std::vector<int> serialSGSCore(const std::vector<int>& order, Matrix<int> &resRem, bool robust = false) const;

    bool enoughCapacityForJob(int job, int t, Matrix<int> & resRem) const;

    int computeFirstSuccStartingTime(const std::vector<int> &sts, int job) const;
	int computeFirstSuccStartingTimeForPartial(const std::vector<int> &fts, int job) const;
    int computeLastPredFinishingTimeForPartial(const std::vector<int> &fts, int job) const;

    void scheduleJobAt(int job, int t, std::vector<int> &sts, std::vector<int> &fts, Matrix<int> &resRem) const;
	void scheduleJobAt(int job, int t, std::vector<int> &sts, Matrix<int> &resRem) const;

	void unscheduleJob(int j, std::vector<int>& sts, std::vector<int>& fts, Matrix<int>& resRem) const;
	void unscheduleJob(int j, std::vector<int>& sts, Matrix<int>& resRem) const;

	SGSResult earliestStartSchedule() const;

    void transferAlreadyScheduled(std::vector<int> &destSts, const std::vector<int> &partialSts) const;
	void transferAlreadyScheduledToFts(std::vector<int> &destFts, const std::vector<int> &partialSts) const;

	void shiftScheduleLeftBy(int offset, std::vector<int> &sts, Matrix<int> &resRem) const;

	std::vector<int> stsToFts(const std::vector<int>& sts) const;

	Project(const std::string &projectName, const std::vector<std::string>& lines);

private:
    void parsePrecedenceRelation(const std::vector<std::string> &lines);
    void parseDurationsAndDemands(const std::vector<std::string> &lines);

    void reorderDispositionMethod();

	template <class Pred>
	std::vector<int> topOrderComputationCore(Pred isEligible) const;
	std::vector<int> computeTopOrder() const;
	std::vector<int> computeReverseTopOrder() const;

    void computeELSFTs();

    void computeNodeDepths(int root, int curDepth, std::vector<int> &nodeDepths);

	int heuristicMaxMs;
};

template<class Func>
inline void Project::timeWindow(int j, Func code) const {
    for(int t=efts[j]; t<=lfts[j]; t++) { code(t); }
}

template<class Func>
inline void Project::timeWindowBounded(int j, Func code) const {
	for (int t = efts[j]; t <= std::min(lfts[j], heuristicMaxMs); t++) { code(t); }
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
		for (int t = efts[j]; t <= std::min(lfts[j], heuristicMaxMs); t++) {
			code(j, t);
		}
	});
}

template <class Func>
inline void Project::demandInPeriodMIP(int j, int t, Func code) const {
	for(int tau = t; tau < std::min(t + durations[j], getHeuristicMaxMakespan() + 1); tau++) {
		code(tau);
	}
}

inline int Project::makespan(const std::vector<int>& sts) const {
    return sts[lastJob];
}

inline int Project::getHeuristicMaxMakespan() const {
	return heuristicMaxMs;
}

