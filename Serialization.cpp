#include "Libraries/json11.hpp"
#include "Serialization.h"
#include "ProjectWithOvertime.h"

json11::Json serializeProjectCommon(const Project& p) {
	// TODO: Implement me!
	return json11::Json();
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
	string elems = "";
	for(int i = lb; i <= ub; i++)
		elems += indexPrefix + std::to_string(i) + (i == ub ? "" : ",");
	return "set " + setName + " /" + elems + "/;\n";
}
