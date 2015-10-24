//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_PROJECTWITHOVERTIME_H
#define SSGS_PROJECTWITHOVERTIME_H


#include "Project.h"

class ProjectWithOvertime : public Project {
public:
    vector<int> zmax;
    vector<float> revenue, kappa;
    vector<int> zeroOc;

    ProjectWithOvertime(string filename);

private:
    void computeRevenueFunction();
    int computeTKappa() const;
    vector<int> earliestStartSchedule(vector<vector<int>> & resRem);
    float totalCosts(vector<vector<int>> resRem);
};


#endif //SSGS_PROJECTWITHOVERTIME_H
