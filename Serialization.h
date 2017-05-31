#pragma once

#include <string>
#include <vector>

#include "Matrix.h"
#include "Libraries/json11.hpp"

class Project;
class ProjectWithOvertime;

namespace Serialization {

	namespace JSON {
		template<class T>
		json11::Json matrixToJson(const Matrix<T> &m) {
			std::vector<std::vector<T>> rows(m.getM());
			for(int i=0; i<m.getM(); i++) {
				rows[i] = m.row(i);
			}
			return rows;
		}

		std::string serializeProject(const Project &p);
		std::string serializeProject(const ProjectWithOvertime &p);
		std::string serializeSchedule(const std::vector<int> &sts);
	}

	namespace GAMS {
		std::string serializeSet(std::string setName, std::string indexPrefix, int lb, int ub);

		template<class T>
		std::string serializeParam1D(std::string setDepName, std::string indexPrefix, std::vector<T> &elements) {
			// TODO: Implement me!
		}

		template<class T>
		std::string serializeParam2D(std::string setDepName1, std::string setDepName2, std::string indexPrefix1, std::string indexPrefix2, Matrix<T> &elements) {
			// TODO: Implement me!
		}
	}
	
};