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
	SGSDeadlineResult(bool _valid, vector<int> _sts, Matrix<int> _resRem) : SGSResult(_sts, _resRem), valid(_valid)  {}
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
    SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, bool robust = false) const;
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

private:
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;	
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
	static int nthDecisionTimeWithMinCosts(int nth, vector<int> &decisionTimes, vector<float> &assocExtCosts, float minCosts);
};


#endif //SSGS_PROJECTWITHOVERTIME_H
