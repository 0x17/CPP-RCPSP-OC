//
// Created by André Schnabel on 21.12.17.
//

#pragma once

#include <cassert>
#include "Libraries/json11.hpp"

template<class T>
class Matrix;

namespace JsonUtils {

    json11::Json readJsonFromString(const std::string &s);
    json11::Json readJsonFromFile(const std::string &filename);

    Matrix<int> matrixIntFromJson(const json11::Json &obj);
    json11::Json::array matrixIntToJson(const Matrix<int> &mx);

    class IJsonSerializable {
    public:
        virtual ~IJsonSerializable() = default;
        virtual json11::Json to_json() const = 0;
        virtual void from_json(const json11::Json &obj) = 0;

        virtual void to_disk(const std::string &filename);
        virtual void from_disk(const std::string &filename, bool skipOnMissing = false);

		void print() const;
    };

    json11::Json modifyObject(const json11::Json& obj, const std::string& key, const json11::Json& newValue);
    json11::Json mergeObjects(const json11::Json &a, const json11::Json &b);

    std::vector<float> extractNumberArrayFromObj(const json11::Json& obj, const std::string& key);
    std::vector<int> extractIntArrayFromObj(const json11::Json& obj, const std::string& key);

    void fillIntFieldsWithObject(const json11::Json& obj, const std::vector<std::string> &keys, const std::vector<int *> &fields);
    void fillStrFieldsWithObject(const json11::Json& obj, const std::vector<std::string> &keys, const std::vector<std::string *> &fields);

	template<class T>
	void assignNumberSlotsFromJsonWithMapping(const json11::Json &obj, const std::map<std::string, T *> mapping) {
		for (const auto &pair : mapping) {
			if (obj[pair.first].is_number()) {
				*pair.second = static_cast<T>(obj[pair.first].number_value());
			}
		}
	}

	void assignBooleanSlotsFromJsonWithMapping(const json11::Json &obj, const std::map<std::string, bool *> mapping);
}