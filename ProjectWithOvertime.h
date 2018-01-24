//
// Created by André Schnabel on 23.10.15.
//

#pragma once

#include <map>

#include "Project.h"

class ProjectWithOvertime : public Project {
public:
    std::vector<int> zmax, zzero;
	std::vector<float> kappa, revenue;
	std::vector<float> revenueExtrapolated;

	explicit ProjectWithOvertime(const std::string &filename);
    explicit ProjectWithOvertime(JsonWrap obj);
	ProjectWithOvertime(const std::string& projectName, const std::string& contents);
	ProjectWithOvertime(const std::string& projectName, const std::vector<std::string> &lines);

	~ProjectWithOvertime() final = default;

	float calcProfit(int makespan, const Matrix<int> &resRem) const;
	float calcProfit(const SGSResult& result) const;
	float calcProfit(const std::vector<int> &sts) const;

	float totalCosts(const Matrix<int> & resRem) const;
	float totalCosts(const std::vector<int> &sts) const;
	float totalCosts(const SGSResult& result) const;
	float totalCostsForPartial(const std::vector<int> &sts) const;

    SGSResult serialSGSWithOvertime(const std::vector<int> &order, bool robust = false) const;
	SGSResult serialSGSWithOvertimeWithForwardBackwardImprovement(const std::vector<int>& order, bool robust = false) const;

	// START (lambda|beta)
	struct BorderSchedulingOptions {
		bool separateCrossover, assocIndex, upper;

		BorderSchedulingOptions();
		BorderSchedulingOptions(bool _separateCrossover, bool _assocIndex, bool _upper);
		explicit BorderSchedulingOptions(int ix);

		void setFromIndex(int ix);
	};
	struct PartialScheduleData {
		Matrix<int> resRem;
		std::vector<int> sts, fts;
		explicit PartialScheduleData(ProjectWithOvertime const* p);
	};
	struct ResidualData {
		Matrix<int> normal, overtime;
		explicit ResidualData(ProjectWithOvertime const* p);
	};

	void scheduleJobSeparateResiduals(int job, int t, int bval, PartialScheduleData& data, ResidualData& residuals) const;
	void scheduleJobBorderLower(int job, int lastPredFinished, int bval, PartialScheduleData& data) const;
	void scheduleJobBorderUpper(int job, int lastPredFinished, int bval, PartialScheduleData& data, ResidualData& residuals) const;
	SGSResult serialSGSTimeWindowBorders(const std::vector<int> &order, const std::vector<int> &beta, const BorderSchedulingOptions &options, bool robust = false) const;
	SGSResult serialSGSTimeWindowBordersWithForwardBackwardImprovement(const std::vector<int> &order, const std::vector<int> &beta, const BorderSchedulingOptions &options, bool robust = false) const;
	// END (lambda|beta)

	SGSResult serialSGSWithForwardBackwardImprovement(const std::vector<int>& order, const std::vector<int>& z, bool robust = false) const;
	SGSResult serialSGSWithForwardBackwardImprovement(const std::vector<int>& order, const Matrix<int>& z, bool robust = false) const;

	SGSResult serialSGSWithRandomKeyAndFBI(const std::vector<float> &rk, const Matrix<int>& z) const;
	SGSResult serialSGSWithRandomKeyAndFBI(const std::vector<float> &rk, const std::vector<int>& z) const;

    SGSResult serialSGSTimeWindowArbitrary(const std::vector<int> &order, const std::vector<float> &tau, bool robust = false) const;
	SGSResult serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(const std::vector<int> &order, const std::vector<float> &tau, bool robust = false) const;

	std::vector<int> earliestStartingTimesForPartialRespectZmax(const std::vector<int> &sts, const Matrix<int> &resRem) const;

	int heuristicMakespanUpperBound() const;

	SGSResult forwardBackwardDeadlineOffsetSGS(const std::vector<int> &order, int deadlineOffset, bool robust = false) const;
	SGSResult delayWithoutOvertimeIncrease(const std::vector<int>& order, const std::vector<int>& baseSts, const Matrix<int>& baseResRem, int deadline, bool robust = false) const;
	SGSResult earlierWithoutOvertimeIncrease(const std::vector<int>& order, const std::vector<int>& baseSts, const Matrix<int>& baseResRem, bool robust = false) const;
	boost::optional<float> costsAndFeasibilityCausedByActivity(int j, int stj, const Matrix<int>& resRem) const;
	int latestCheapestFeasiblePeriod(int j, int baseStj, int lstj, const Matrix<int>& resRem) const;
	int earliestCheapestFeasiblePeriod(int j, int baseStj, int estj, const Matrix<int>& resRem) const;
	SGSResult goldenSectionSearchBasedOptimization(const std::vector<int>& order, bool robust = false) const;
	SGSResult forwardBackwardIterations(const std::vector<int> &order, SGSResult result, int deadline, boost::optional<int> numIterations = boost::optional<int>(), bool robust = false) const;

	bool isScheduleResourceFeasible(const std::vector<int>& sts) const override;

	std::map<int, std::pair<int, float>> heuristicProfitsAndActualMakespanForRelevantDeadlines(const std::vector<int> &order) const;

	json11::Json to_json() const override;
	void from_json(const json11::Json& obj) override;

	std::vector<int> serialOptimalSubSGS(const std::vector<int>& partitions, int partitionSize) const;
	std::vector<int> serialOptimalSubSGSWithPartitionList(const std::vector<int>& partitionList) const;

	bool isPartitionListFeasible(const std::vector<int> &partitionList, int partitionSize) const;

	std::vector<int> serialOptimalSubSGSAndFBI(const std::vector<int>& partitions, int partitionSize) const;
	std::vector<int> serialOptimalSubSGSWithPartitionListAndFBI(const std::vector<int>& partitionList) const;

	SGSResult parallelSGSWithForwardBackwardImprovement(const std::vector<int> &order, const Matrix<int> &z) const;
	SGSResult parallelSGSWithForwardBackwardImprovement(const std::vector<int> &order, const std::vector<int> &z) const;
	SGSResult parallelSGSWithForwardBackwardImprovement(const std::vector<int> &order) const;

private:
    void computeRevenueFunction();
	void computeExtrapolatedRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;
};
