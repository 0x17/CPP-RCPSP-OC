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
    bool enoughCapacityForJobWithOvertime(int job, int t, Matrix<int> & resRem) const;
    void computeRevenueFunction();
    int computeTKappa() const;
    vector<int> earliestStartSchedule(Matrix<int> & resRem);
};


#endif //SSGS_PROJECTWITHOVERTIME_H
