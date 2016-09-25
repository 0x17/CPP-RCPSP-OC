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
	virtual ~ProjectWithOvertime() {}

	float calcProfit(int makespan, const Matrix<int> &resRem) const;
	float calcProfit(const SGSResult& result) const;
	float calcProfit(const vector<int> &sts) const;

	float totalCosts(const Matrix<int> & resRem) const;
	float totalCosts(const vector<int> &sts) const;
	float totalCosts(const SGSResult& result) const;
	float totalCostsForPartial(const vector<int> &sts) const;

    SGSResult serialSGSWithOvertime(const vector<int> &order, bool robust = false) const;

	// START (lambda|beta)
	struct BorderSchedulingOptions {
		bool separateCrossover, linked, upper;

		BorderSchedulingOptions();
		BorderSchedulingOptions(bool _separateCrossover, bool _linked, bool _upper);
		BorderSchedulingOptions(int ix);

		void setFromIndex(int ix);
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
	SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, BorderSchedulingOptions options, bool robust = false) const;
	// END (lambda|beta)

    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust = false) const;
	bool enoughCapacityForJobWithBaseInterval(const vector<int> & sts, const vector<int> & cests, const vector<int> & clfts, const Matrix<int> & resRem, int j, int stj) const;

	vector<int> earliestStartingTimesForPartialRespectZmax(const vector<int> &sts, const Matrix<int> &resRem) const;

	vector<int> decisionTimesForResDevProblem(const vector<int> &sts, const vector<int> &ests, const vector<int> &lfts, const Matrix<int> &resRem, int j) const;

	static vector<int> jobsWithDescendingStartingTimes(const vector<int>& sts);
	list<int> feasibleTimeWindowForJobInCompleteSchedule(int j, const vector<int>& sts, const vector<int>& fts, const Matrix<int>& resRem) const;
	int latestPeriodWithMinimalCosts(int j, const list<int>& feasTimes, const vector<int>& sts, const Matrix<int>& resRem) const;

	int heuristicMakespanUpperBound() const;

	SGSResult earlyOvertimeDeadlineOffsetSGS(const vector<int> &order, int deadlineOffset, bool robust = false) const;
	SGSResult delayWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust = false) const;
	SGSResult earlierWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, bool robust = false) const;
	boost::optional<float> costsAndFeasibilityCausedByActivity(int j, int stj, const Matrix<int>& resRem) const;
	int latestCheapestFeasiblePeriod(int j, int baseStj, int lstj, const Matrix<int>& resRem) const;
	int earliestCheapestFeasiblePeriod(int j, int baseStj, int estj, const Matrix<int>& resRem) const;
	SGSResult forwardBackwardWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust = false) const;
	SGSResult goldenSectionSearchBasedOptimization(const vector<int>& order, bool robust = false) const;

	bool isScheduleResourceFeasible(const vector<int>& sts) const override;

private:
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;	
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
};


#endif //SSGS_PROJECTWITHOVERTIME_H
