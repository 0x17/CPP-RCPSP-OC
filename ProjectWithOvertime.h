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

	explicit ProjectWithOvertime(const std::string &filename);
    explicit ProjectWithOvertime(JsonWrap _obj);
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
	
	int heuristicMakespanUpperBound() const;

	bool isScheduleResourceFeasible(const std::vector<int>& sts) const override;
	
	json11::Json to_json() const override;
	void from_json(const json11::Json& obj) override;

private:
    void computeRevenueFunction();
    int computeTKappa() const;
	bool enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const;
};
