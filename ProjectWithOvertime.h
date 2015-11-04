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

private:
    void computeRevenueFunction();
    int computeTKappa() const;
    vector<int> earliestStartSchedule(vector<vector<int>> & resRem);    
};


#endif //SSGS_PROJECTWITHOVERTIME_H
