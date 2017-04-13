#pragma once

#include <string>
#include <vector>

#include "Matrix.h"

using std::string;
using std::vector;

class Project;
class ProjectWithOvertime;

namespace Serialization {

	namespace JSON {
		string serializeProject(const Project &p);
		string serializeProject(const ProjectWithOvertime &p);
		string serializeSchedule(const vector<int> &sts);
	}

	namespace GAMS {
		string serializeSet(string setName, string indexPrefix, int lb, int ub);

		template<class T>
		string serializeParam1D(string setDepName, string indexPrefix, vector<T> &elements) {
			// TODO: Implement me!
		}

		template<class T>
		string serializeParam2D(string setDepName1, string setDepName2, string indexPrefix1, string indexPrefix2, Matrix<T> &elements) {
			// TODO: Implement me!
		}
	}
	
};