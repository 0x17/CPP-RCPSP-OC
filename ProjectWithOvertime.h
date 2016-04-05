//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECTWITHOVERTIME_H
#define SSGS_PROJECTWITHOVERTIME_H

#include <list>
#include "Project.h"

class ProjectWithOvertime : public Project {
public:
    vector<int> zmax;
	vector<float> kappa, revenue;

	explicit ProjectWithOvertime(string filename);

	float calcProfit(int makespan, const Matrix<int> &resRem) const;
	float calcProfit(const vector<int> &sts) const;

	float totalCosts(const Matrix<int> & resRem) const;
	float totalCosts(const vector<int> &sts) const;
    float totalCostsForPartial(const vector<int> &sts) const;

    SGSResult serialSGSWithOvertime(const vector<int> &order, bool robust = false) const;
    SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, bool robust = false) const;
    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust = false) const;
	bool enoughCapacityForJobWithBaseInterval(const vector<int> & sts, const vector<int> & cests, const vector<int> & clfts, const Matrix<int> & resRem, int j, int stj) const;

	template<class Func>
	pair<bool, SGSResult> serialSGSWithDeadline(int deadline, const vector<int> &order, Func chooseIndex) const;

	pair<bool, SGSResult> serialSGSWithDeadlineEarly(int deadline, const vector<int>& order) const;
	pair<bool, SGSResult> serialSGSWithDeadlineLate(int deadline, const vector<int>& order) const;
	pair<bool, SGSResult> serialSGSWithDeadlineBeta(int deadline, const vector<int>& order, const vector<int>& beta) const;

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
