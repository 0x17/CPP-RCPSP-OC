//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include <string>
#include <map>

#include <boost/algorithm/clamp.hpp>

#include "ProjectWithOvertime.h"

#include "JobPriorityProviders.h"
#include "OvertimeChoiceProviders.h"

using namespace std;

ProjectWithOvertime::ProjectWithOvertime(JsonWrap _obj) : Project(_obj) {
	auto obj = _obj.obj;
    revenue = JsonUtils::extractNumberArrayFromObj(obj, "u");
    kappa = JsonUtils::extractNumberArrayFromObj(obj, "kappa");
    zmax = JsonUtils::extractIntArrayFromObj(obj, "zmax");
}

ProjectWithOvertime::ProjectWithOvertime(const string &filename) :
	ProjectWithOvertime(boost::filesystem::path(filename).stem().string(), Utils::readLines(filename)) {}

ProjectWithOvertime::ProjectWithOvertime(const string& projectName, const string& contents) :
	ProjectWithOvertime(projectName, Utils::splitLines(contents)) {}

ProjectWithOvertime::ProjectWithOvertime(const string& projectName, const vector<string>& lines) :
	Project(projectName, lines),
	zmax(numRes),
	zzero(numRes, 0),
	kappa(numRes),
	revenue(numPeriods) {
	eachRes([&](int r) {
		zmax[r] = capacities[r] / 2;
		kappa[r] = 0.5f;
	});
	computeRevenueFunction();
}

inline float ProjectWithOvertime::totalCosts(const Matrix<int> & resRem) const {
	float costs = 0.0f;
    EACH_RES_PERIOD(costs += Utils::max(0, -resRem(r,t)) * kappa[r])
	return costs;
}

float ProjectWithOvertime::totalCosts(const vector<int> &sts) const {
	float costs = 0.0f;
	int cdemand;
	EACH_RES_PERIOD(
		cdemand = 0;
		EACH_JOB(if (sts[j] < t && t <= sts[j] + durations[j]) cdemand += demands(j, r))
		costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
    )
	return costs;
}

float ProjectWithOvertime::totalCosts(const SGSResult& result) const {
	return totalCosts(result.resRem);
}

float ProjectWithOvertime::totalCostsForPartial(const vector<int> &sts) const {
    float costs = 0.0f;
    int cdemand;
    EACH_RES_PERIOD(
        cdemand = 0;
        EACH_JOB(if (sts[j] != UNSCHEDULED && sts[j] < t && t <= sts[j] + durations[j]) cdemand += demands(j, r))
        costs += Utils::max(0, cdemand - capacities[r]) * kappa[r];
    )
    return costs;
}

float ProjectWithOvertime::calcProfit(const vector<int> &sts) const {
	return revenue[makespan(sts)] - totalCosts(sts);
}

float ProjectWithOvertime::calcProfit(int makespan, const Matrix<int>& resRem) const {
	return revenue[makespan] - totalCosts(resRem);
}

float ProjectWithOvertime::calcProfit(const SGSResult& result) const {
	return calcProfit(makespan(result.sts), result.resRem);
}

void ProjectWithOvertime::computeRevenueFunction() {
	const auto ess = earliestStartSchedule();

	// minMs previously defined as: Utils::max(makespan(ess), computeTKappa());
	const int minMs = makespan(ess);
    const int maxMs = makespan(SerialScheduleGenerationScheme(*this).constructSchedule(al(topOrder), ocnone()));

	const float maxCosts = totalCosts(ess);

    EACH_PERIOD(revenue[t] = static_cast<float>(
		(minMs >= maxMs || t < minMs) ? maxCosts :
		(t > maxMs) ? 0.0f :
		maxCosts - maxCosts / pow(maxMs-minMs, 2) * pow(t-minMs, 2)))
}

int ProjectWithOvertime::computeTKappa() const {
    int tkappa = 0;
    EACH_RES(
		float tkappar = 0.0f;
        EACH_JOB(tkappar += durations[j] * demands(j,r))
		tkappar /= static_cast<float>(capacities[r] + zmax[r]);
		tkappar = ceil(tkappar);
		tkappa = Utils::max(tkappa, static_cast<int>(tkappar));
    )
    return tkappa;
}


json11::Json ProjectWithOvertime::to_json() const {
	const auto rocObj = json11::Json::object {
		{"u", revenue},
		{"kappa", kappa},
		{"zmax", zmax}
	};
	return JsonUtils::mergeObjects(Project::to_json(), rocObj);
}

void ProjectWithOvertime::from_json(const json11::Json& obj) {
	Project::from_json(obj);
	revenue = JsonUtils::extractNumberArrayFromObj(obj, "u");
	kappa = JsonUtils::extractNumberArrayFromObj(obj, "kappa");
	zmax = JsonUtils::extractIntArrayFromObj(obj, "zmax");
}

bool ProjectWithOvertime::isScheduleResourceFeasible(const vector<int>& sts) const {
	return Project::isScheduleResourceFeasible(sts, zmax);
}

bool ProjectWithOvertime::enoughCapacityForJobWithOvertime(int job, int t, const Matrix<int> & resRem) const {
    ACTIVE_PERIODS(job, t, EACH_RES(if(demands(job,r) > resRem(r,tau) + zmax[r]) return false))
    return true;
}


