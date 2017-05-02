#pragma once

#include <string>
#include <vector>

#include "Matrix.h"
#include "Libraries/json11.hpp"

using std::string;
using std::vector;

class Project;
class ProjectWithOvertime;

namespace Serialization {

	namespace JSON {
		template<class T>
		json11::Json matrixToJson(const Matrix<T> &m) {
			vector<vector<T>> rows(m.getM());
			for(int i=0; i<m.getM(); i++) {
				rows[i] = m.row(i);
			}
			return rows;
		}

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