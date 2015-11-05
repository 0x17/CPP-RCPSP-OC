//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECTWITHOVERTIME_H
#define SSGS_PROJECTWITHOVERTIME_H


#include "Project.h"

class ProjectWithOvertime : public Project {
public:
    vector<int> zmax;
	vector<float> kappa, revenue;

	explicit ProjectWithOvertime(string filename);

	float calcProfit(int makespan, const vector<vector<int>> &resRem);
	float totalCosts(const vector<vector<int>> & resRem);

    SGSResult serialSGSWithOvertime(vector<int> order);
    SGSResult serialSGSTimeWindowBorders(vector<int> order, vector<int> beta);
    SGSResult serialSGSTimeWindowArbitrary(vector<int> order, vector<float> tau);

private:
    bool enoughCapacityForJobWithOvertime(int job, int t, vector<vector<int>> & resRem) const;
    void computeRevenueFunction();
    int computeTKappa() const;
    vector<int> earliestStartSchedule(vector<vector<int>> & resRem);    
};


#endif //SSGS_PROJECTWITHOVERTIME_H
