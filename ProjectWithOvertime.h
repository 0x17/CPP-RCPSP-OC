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

    SGSResult serialSGSWithOvertime(const vector<int> &order);
    SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta);
    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau);
    SGSResult serialSGSWithDeadline(int deadline, const vector<int> order);

	vector<int> earliestStartingTimesForPartial(const vector<int> &order) const;

private:
    void computeRevenueFunction();
    int computeTKappa();    

	bool enoughCapacityForJobWithOvertime(int job, int t, Matrix<int> & resRem) const;	
	vector<int> latestFinishingTimesForPartial(const vector<int> &order) const;
	list<int> decisionTimesForResDevProblem(const vector<int> &sts, const vector<int> &ests, const vector<int> &lfts, int j) const;
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
	int selectBestStartingTime(vector<int> &sts, int j, const list<int> &decisionTimes, const Matrix<int> &resRem) const;
};


#endif //SSGS_PROJECTWITHOVERTIME_H
