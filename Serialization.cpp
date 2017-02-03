#include "Serialization.h"
#include "Libraries/json11.hpp"
#include "ProjectWithOvertime.h"

json11::Json serializeProjectCommon(const Project& p) {
	// TODO: Implement me!
	return json11::Json();
}

std::string Serialization::JSON::serializeProject(const Project &p) {
	return serializeProjectCommon(p).dump();
}

std::string Serialization::JSON::serializeProject(const ProjectWithOvertime& p) {
	json11::Json::object obj = serializeProjectCommon(p).object_items();
	obj["kappa"] = p.kappa;
	obj["zmax"] = p.zmax;
	obj["u"] = p.revenue;
	return json11::Json(obj).dump();
}

std::string Serialization::JSON::serializeSchedule(const std::vector<int>& sts) {
	return json11::Json(sts).dump();
}

std::string Serialization::GAMS::serializeSet(std::string setName, std::string indexPrefix, int lb, int ub) {
	std::string elems = "";
	for(int i = lb; i <= ub; i++)
		elems += indexPrefix + std::to_string(i) + (i == ub ? "" : ",");
	return "set " + setName + " /" + elems + "/;\n";
}
