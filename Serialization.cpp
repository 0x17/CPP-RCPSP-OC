#include "Libraries/json11.hpp"
#include "Serialization.h"
#include "ProjectWithOvertime.h"

json11::Json serializeProjectCommon(const Project& p) {
	json11::Json obj = json11::Json::object {
			{"instanceName", p.instanceName},
			{"numJobs",      p.numJobs},
			{"numRes",       p.numRes},
			{"numPeriods",   p.numPeriods},
			{"capacities",   p.capacities},
			{"durations",    p.durations},
			{"demands",      Serialization::JSON::matrixToJson<int>(p.demands)},
			{"adjMx",        Serialization::JSON::matrixToJson<char>(p.adjMx)}
	};
	return obj;
}

string Serialization::JSON::serializeProject(const Project &p) {
	return serializeProjectCommon(p).dump();
}

string Serialization::JSON::serializeProject(const ProjectWithOvertime& p) {
	json11::Json::object obj = serializeProjectCommon(p).object_items();
	obj["kappa"] = p.kappa;
	obj["zmax"] = p.zmax;
	obj["u"] = p.revenue;
	return json11::Json(obj).dump();
}

string Serialization::JSON::serializeSchedule(const vector<int>& sts) {
	return json11::Json(sts).dump();
}

string Serialization::GAMS::serializeSet(string setName, string indexPrefix, int lb, int ub) {
	return "set " + setName + " /" + indexPrefix + std::to_string(lb) + ".." + indexPrefix + std::to_string(ub) + "/;\n";
}
