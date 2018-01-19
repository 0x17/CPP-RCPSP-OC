//
// Created by André Schnabel on 21.12.17.
//

#include "JsonUtils.h"
#include "Utils.h"

#include <string>
#include <map>

using namespace std;
using namespace json11;

Json JsonUtils::readJsonFromString(const std::string &s) {
    std::string err;
    auto obj = Json::parse(s, err);
    if(!err.empty()) {
        throw new std::runtime_error("Parse error: " + err);
    }
    return obj;
}

Json JsonUtils::readJsonFromFile(const std::string &filename) {
    return readJsonFromString(Utils::slurp(filename));
}

Json JsonUtils::modifyObject(const Json& obj, const string& key, const Json& newValue) {
    if(obj.is_object()) {
        map<string, Json> mutableMapping = obj.object_items();
        mutableMapping[key] = newValue;
        return Json{mutableMapping};
    }
    return obj;
}

Json JsonUtils::mergeObjects(const Json& a, const Json& b) {
    if (a.is_object() && b.is_object()) {
        Json merged = a;
        for (const auto item : b.object_items()) {
            merged = modifyObject(merged, item.first, item.second);
        }
        return merged;
    }
    return Json::object{};
}

std::vector<float> JsonUtils::extractNumberArrayFromObj(const Json& obj, const std::string& key) {
    auto arr = obj[key].array_items();
    std::vector<float> nums(arr.size());
    for(int i = 0; i < arr.size(); i++)
        nums[i] = (float)arr[i].number_value();
    return nums;
}

std::vector<int> JsonUtils::extractIntArrayFromObj(const Json& obj, const std::string& key) {
    auto arr = obj[key].array_items();
    std::vector<int> nums(arr.size());
    for (int i = 0; i < arr.size(); i++)
        nums[i] = arr[i].int_value();
    return nums;
}

void JsonUtils::fillIntFieldsWithObject(const Json &obj, const std::vector<std::string> &keys, const std::vector<int *> &fields) {
    for(int i=0; i<keys.size(); i++) {
        assert(obj[keys[i]].is_number());
        *(fields[i]) = obj[keys[i]].int_value();
    }
}

void JsonUtils::fillStrFieldsWithObject(const Json &obj, const std::vector<std::string> &keys, const std::vector<std::string *> &fields) {
    for(int i=0; i<keys.size(); i++) {
        assert(obj[keys[i]].is_string());
        *(fields[i]) = obj[keys[i]].string_value();
    }
}

Matrix<int> JsonUtils::matrixIntFromJson(const Json &obj) {
    assert(obj.is_array());
    auto rows = obj.array_items();
    int nrows = static_cast<int>(rows.size());

    assert(rows[0].is_array());
    int ncols = static_cast<int>(rows[0].array_items().size());

    Matrix<int> res(nrows, ncols);

    for(int i=0; i<nrows; i++) {
        assert(rows[i].is_array());
        auto row = rows[i].array_items();
        assert(row.size() == ncols);

        for(int j=0; j<row.size(); j++) {
            res(i,j) = row[j].int_value();
        }
    }

    return res;
}

Json::array JsonUtils::matrixIntToJson(const Matrix<int> &mx) {
    return Utils::constructVector<Json>(mx.getM(), [&mx](int i) {
        return Utils::constructVector<Json>(mx.getN(), [&mx,i](int j) {
            return mx.at(i,j);
        });
    });
}

void JsonUtils::IJsonSerializable::to_disk(const std::string &filename) {
    Utils::spit(to_json().dump(), filename);
}

void JsonUtils::IJsonSerializable::from_disk(const std::string &filename) {
    from_json(JsonUtils::readJsonFromFile(filename));
}
