//
// Created by André Schnabel on 10.10.18.
//

#include "SensitivityAnalysis.h"

#include <boost/algorithm/string/join.hpp>

using namespace std;

void sensitivity::varyTotalAvailableCapacity(const std::string &fn, int r) {
	const auto modfunc = [r](ProjectWithOvertime *p, float v) {
		int vi = static_cast<int>(v);
		p->zmax[r] = max(0, vi-p->capacities[r]);
		p->capacities[r] = min(p->capacities[r], vi);
		p->updateDerivedParameters();
	};

	ProjectWithOvertime p(fn);

	float maxDemand = 0.0f;
	p.eachJobConst([&](int j) {
		if(p.demands(j, r) > maxDemand)
			maxDemand = p.demands(j, r);
	});

	float maxCapWithOC = p.capacities[r] + p.zmax[r];

	const auto results = collectResultsForRange(modfunc, fn, maxDemand, maxCapWithOC, 1.0f);
	resultsToCsvFile(p, results, "varycapacity.csv");
}

void sensitivity::resultsToCsvFile(const ProjectWithOvertime &p, const list<sensitivity::ResultForValue> &results, const std::string &ofn) {
	const auto statMap = p.scheduleStatistics(p.serialSGS(p.topOrder));

	const auto constructHeader = [&statMap]() {
		vector<string> stats;
		for(const auto &pair : statMap) {
			stats.push_back(pair.first);
		}
		return "totalCapacity;"+boost::algorithm::join(stats, ";")+"\n";
	};

	std::stringstream ss;
	for(const auto &res : results) {
		const auto itsStats = p.scheduleStatistics(res.sts);
		vector<string> vals;
		for(const auto &pair : itsStats) {
			vals.push_back(to_string(pair.second));
		}
		ss << res.value << ";" << boost::algorithm::join(vals, ";") << endl;
	}

	Utils::spit(constructHeader() + ss.str(), ofn);
}
