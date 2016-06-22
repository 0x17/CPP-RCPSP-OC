//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECTWITHOVERTIME_H
#define SSGS_PROJECTWITHOVERTIME_H

#include <list>
#include "Project.h"

//typedef pair<bool, SGSResult> SGSDeadlineResult;
struct SGSDeadlineResult : SGSResult {
	bool valid;
	SGSDeadlineResult(bool _valid, vector<int> _sts, Matrix<int> _resRem) : valid(_valid), SGSResult(_sts, _resRem) {}
};

class ProjectWithOvertime : public Project {
public:
    vector<int> zmax;
	vector<float> kappa, revenue;

	explicit ProjectWithOvertime(string filename);

	float calcProfit(int makespan, const Matrix<int> &resRem) const;
	float calcProfit(const SGSResult& result) const;
	float calcProfit(const vector<int> &sts) const;

	float totalCosts(const Matrix<int> & resRem) const;
	float totalCosts(const vector<int> &sts) const;
    float totalCostsForPartial(const vector<int> &sts) const;

    SGSResult serialSGSWithOvertime(const vector<int> &order, bool robust = false) const;

	// START (lambda|beta)
	struct BorderSchedulingOptions {
		bool robust, linked, upper;
		BorderSchedulingOptions();
		BorderSchedulingOptions(bool _robust, bool _linked, bool _upper);
	};
	struct PartialScheduleData {
		Matrix<int> resRem;
		vector<int> sts, fts;
		PartialScheduleData(ProjectWithOvertime const* p);
	};
	struct ResidualData {
		Matrix<int> normal, overtime;
		ResidualData(ProjectWithOvertime const* p);
	};
	void scheduleJobSeparateResiduals(int job, int t, int bval, PartialScheduleData& data, ResidualData& residuals) const;
	void scheduleJobBorderLower(int job, int lastPredFinished, int bval, PartialScheduleData& data) const;
	void scheduleJobBorderUpper(int job, int lastPredFinished, int bval, PartialScheduleData& data, ResidualData& residuals) const;
	SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, BorderSchedulingOptions options) const;
	// END (lambda|beta)

    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust = false) const;
	bool enoughCapacityForJobWithBaseInterval(const vector<int> & sts, const vector<int> & cests, const vector<int> & clfts, const Matrix<int> & resRem, int j, int stj) const;

	template<class Func>
	SGSDeadlineResult serialSGSWithDeadline(int deadline, const vector<int> &order, Func chooseIndex) const;

	SGSDeadlineResult serialSGSWithDeadlineEarly(int deadline, const vector<int>& order) const;
	SGSDeadlineResult serialSGSWithDeadlineLate(int deadline, const vector<int>& order) const;
	SGSDeadlineResult serialSGSWithDeadlineBeta(int deadline, const vector<int>& order, const vector<int>& beta) const;
	SGSDeadlineResult serialSGSWithDeadlineTau(int deadline, const vector<int>& order, const vector<float>& tau) const;

	vector<int> earliestStartingTimesForPartialRespectZmax(const vector<int> &sts, const Matrix<int> &resRem) const;

	vector<int> decisionTimesForResDevProblem(const vector<int> &sts, const vector<int> &ests, const vector<int> &lfts, const Matrix<int> &resRem, int j) const;

	static vector<int> jobsWithDescendingStartingTimes(const vector<int>& sts);
	list<int> feasibleTimeWindowForJobInCompleteSchedule(int j, const vector<int>& sts, const vector<int>& fts, const Matrix<int>& resRem) const;
	int latestPeriodWithMinimalCosts(int j, const list<int>& feasTimes, const vector<int>& sts, const Matrix<int>& resRem) const;
	void unscheduleJob(int j, vector<int>& sts, vector<int>& fts, Matrix<int>& resRem);
	void improvementStep(vector<int>& sts);

	int heuristicMakespanUpperBound() const;

private:
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;	
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
	static int nthDecisionTimeWithMinCosts(int nth, vector<int> &decisionTimes, vector<float> &assocExtCosts, float minCosts);
};


#endif //SSGS_PROJECTWITHOVERTIME_H
