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

	float calcProfit(int makespan, const Matrix<int> &resRem);
	float totalCosts(const Matrix<int> & resRem);

    SGSResult serialSGSWithOvertime(const vector<int> &order);
    SGSResult serialSGSTimeWindowBorders(const vector<int> &order, const vector<int> &beta);
    SGSResult serialSGSTimeWindowArbitrary(const vector<int> &order, const vector<float> &tau);
    SGSResult serialSGSWithDeadline(int deadline, const vector<int> order);

private:
    void computeRevenueFunction();
    int computeTKappa();    

	bool enoughCapacityForJobWithOvertime(int job, int t, Matrix<int> & resRem) const;
	vector<int> earliestStartingTimesForPartial(const vector<int> &order) const;
	vector<int> latestFinishingTimesForPartial(const vector<int> &order) const;
	list<int> decisionTimesForResDevProblem(const vector<int> &sts, const vector<int> &ests, const vector<int> &lfts) const;
	float extensionCosts(const Matrix<int> &resRem, int j, int stj) const;
	int selectBestStartingTime(vector<int> &sts, int j, const list<int> &decisionTimes) const;
};


#endif //SSGS_PROJECTWITHOVERTIME_H
