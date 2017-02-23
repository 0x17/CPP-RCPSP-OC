//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECTWITHOVERTIME_H
#define SSGS_PROJECTWITHOVERTIME_H

#include <list>
#include <map>
#include "Project.h"

//typedef pair<bool, SGSResult> SGSDeadlineResult;
struct SGSDeadlineResult : SGSResult {
	bool valid;
	SGSDeadlineResult(bool _valid, vector<int> _sts, Matrix<int> _resRem) : valid(_valid), SGSResult(_sts, _resRem) {}
};

class ProjectWithOvertime : public Project {
public:
    vector<int> zmax, zzero;
	vector<float> kappa, revenue;

	explicit ProjectWithOvertime(const string &filename);
	ProjectWithOvertime(const string& projectName, const string& s);
	ProjectWithOvertime(const string& projectName, const vector<string> &lines);

	virtual ~ProjectWithOvertime() {}

	float calcProfit(int makespan, const Matrix<int> &resRem) const;
	float calcProfit(const SGSResult& result) const;
	float calcProfit(const vector<int> &sts) const;

	float totalCosts(const Matrix<int> & resRem) const;
	float totalCosts(const vector<int> &sts) const;
	float totalCosts(const SGSResult& result) const;
	float totalCostsForPartial(const vector<int> &sts) const;

    SGSResult serialSGSWithOvertime(const vector<int> &order, bool robust = false) const;
	SGSResult serialSGSWithOvertimeWithForwardBackwardImprovement(const vector<int>& order, bool robust = false) const;

	// START (lambda|beta)
	struct BorderSchedulingOptions {
		bool separateCrossover, assocIndex, upper;

		BorderSchedulingOptions();
		BorderSchedulingOptions(bool _separateCrossover, bool _assocIndex, bool _upper);
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

	SGSResult serialSGSTimeWindowBordersWithForwardBackwardImprovement(const vector<int>& order, const vector<int>& beta, BorderSchedulingOptions options, bool robust = false) const;
	// END (lambda|beta)

	SGSResult serialSGSWithForwardBackwardImprovement(const vector<int>& order, const vector<int>& z, bool robust = false) const;
	SGSResult serialSGSWithForwardBackwardImprovement(const vector<int>& order, const Matrix<int>& z, bool robust = false) const;

    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust = false) const;
	SGSResult serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(const vector<int> &order, const vector<float> &tau, bool robust = false) const;

	vector<int> earliestStartingTimesForPartialRespectZmax(const vector<int> &sts, const Matrix<int> &resRem) const;

	int heuristicMakespanUpperBound() const;

	SGSResult forwardBackwardDeadlineOffsetSGS(const vector<int> &order, int deadlineOffset, bool robust = false) const;
	SGSResult delayWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust = false) const;
	SGSResult earlierWithoutOvertimeIncrease(const vector<int>& order, const vector<int>& baseSts, const Matrix<int>& baseResRem, bool robust = false) const;
	boost::optional<float> costsAndFeasibilityCausedByActivity(int j, int stj, const Matrix<int>& resRem) const;
	int latestCheapestFeasiblePeriod(int j, int baseStj, int lstj, const Matrix<int>& resRem) const;
	int earliestCheapestFeasiblePeriod(int j, int baseStj, int estj, const Matrix<int>& resRem) const;
	SGSResult goldenSectionSearchBasedOptimization(const vector<int>& order, bool robust = false) const;
	SGSResult forwardBackwardIterations(const vector<int> &order, SGSResult result, int deadline, boost::optional<int> numIterations = boost::optional<int>(), bool robust = false) const;

	bool isScheduleResourceFeasible(const vector<int>& sts) const override;

	map<int, pair<int, float>> heuristicProfitsAndActualMakespanForRelevantDeadlines(const vector<int> &order) const;

private:
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;
};


#endif //SSGS_PROJECTWITHOVERTIME_H
