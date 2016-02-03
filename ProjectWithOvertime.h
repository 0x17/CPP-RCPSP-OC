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

    SGSResult serialSGSWithOvertime(const vector<int> &order) const;
    SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta, bool robust = false) const;
    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau, bool robust = false) const;
	bool enoughCapacityForJobWithBaseInterval(vector<int> &sts, vector<int> &cests, vector<int> &clfts, Matrix<int> &resRem, int j, int stj) const;
	pair<bool, SGSResult> serialSGSWithDeadline(int deadline, const vector<int> &order) const;
    vector<int> earliestStartingTimesForPartialRespectZmax(const vector<int> &sts, const Matrix<int> &resRem) const;

    int chooseEligibleWithLowestIndex( const vector<int> &sts, const vector<int> &order) const;

private:
	bool allPredsScheduled(int j, const vector<int> &sts) const;
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;
	list<int> decisionTimesForResDevProblem(const vector<int> &sts, const vector<int> &ests, const vector<int> &lfts, int j) const;
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
};


#endif //SSGS_PROJECTWITHOVERTIME_H
