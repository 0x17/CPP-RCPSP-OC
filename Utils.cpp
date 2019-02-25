//
// Created by André Schnabel on 23.10.15.
//

#include <cmath>
#include <regex>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Utils.h"
#include "Stopwatch.h"
#include "Logger.h"

namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

using namespace std;

string Utils::slurp(const string &filename) {
	std::ifstream fp(filename);
	if(!fp) throw std::runtime_error("Unable to open file: " + filename);
	string s((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
	return s;
}

vector<string> Utils::readLines(const string &filename) {
    vector<string> lines;
    string line;
	std::ifstream f(filename);
    if(!f) throw std::runtime_error("Unable to open file: " + filename);
    while(!f.eof()) {
		std::getline(f, line);
        lines.push_back(line);
    }
    return lines;
}

int Utils::extractIntFromStr(const string &s, const string &rx) {
	std::regex rxObj(rx);
	std::smatch result;
	std::regex_search(s, result, rxObj);
    return std::stoi(result[1]);
}

vector<int> Utils::extractIntsFromLine(const string &line) {
    const char delim = ' ';
    vector<int> nums;
    string part;
    for(auto c : line) {
        if(c == delim && !part.empty()) {
            nums.push_back(stoi(part));
            part = "";
        } else if(isdigit(c)) {
            part += c;
        }
    }
    if(!part.empty()) {
        nums.push_back(stoi(part));
    }
    return nums;
}

void Utils::serializeSchedule(const vector<int> &sts, const string &filename) {
	std::ofstream f(filename);
	if(f.is_open()) {
		for (int j = 0; j < sts.size(); j++) {
			f << (j + 1) << "->" << sts[j];
			if (j < sts.size() - 1) f << "\n";
		}
		f.close();
	}
	
}

void Utils::serializeProfit(float profit, const string &filename) {
	spit(std::to_string(profit), filename);
}

int Utils::pickWithDistribution(vector<float> &probs, float q) {
	float cumulatedProbs = 0.0f;
	int lastPossibleIx = 0;
	for(int i = 0; i < probs.size(); i++) {
		if (probs[i] > 0.0f && q >= cumulatedProbs && q <= cumulatedProbs + probs[i])
			return i;
		cumulatedProbs += probs[i];
		if(probs[i] > 0.0f) lastPossibleIx = i;
	}
	return lastPossibleIx;
}

void Utils::spit(const string &s, const string &filename) {
	std::ofstream f(filename);
    if(f.is_open()) {
        f << s;
        f.close();
    }
}

list<string> Utils::filenamesInDir(const string& dir) {
	list<string> fnames;
	fs::path p(dir);
	for (auto it = fs::directory_iterator(p); it != fs::directory_iterator(); ++it) {
		auto entry = *it;
		string filename = entry.path().string();
		if (!fs::is_directory(entry))
			fnames.push_back(filename);

	}
	return fnames;
}

list<string> Utils::filenamesInDirWithExt(const string& dir, const string& ext) {
	list<string> fnames, all;
	fs::path p(dir);
	for (auto it = fs::directory_iterator(p); it != fs::directory_iterator(); ++it) {
		auto entry = *it;
		string filePath = entry.path().string();
		if (!fs::is_directory(entry) && algo::ends_with(filePath, ext)) {
			fnames.push_back(filePath);
		}
	}
	return fnames;
}

void Utils::spitAppend(const string &s, const string &filename) {
    std::ofstream f(filename, std::ios_base::app);
    if(f.is_open()) {
        f << s;
        f.close();
    }
}


string Utils::formattedNow() {
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
	string str(buffer);
	return str;
}

void Utils::partitionDirectory(const string& dirPath, int numPartitions, const string& infix) {
	if(!boost::filesystem::exists(dirPath)) {
		LOG_W("Unable to partition directory " + dirPath + ", it does not exist!");
		return;
	}

	char sep = boost::filesystem::path::preferred_separator;
	list<string> filenames = filenamesInDir(dirPath);

	boost::filesystem::path dirPrefix = boost::filesystem::path(dirPath).remove_trailing_separator();

	auto directoryNameForIx = [&dirPrefix, &infix](int index) {
		return dirPrefix.string() + infix + std::to_string(index);
	};

	auto creatDirForIx = [&directoryNameForIx](int index) {
		boost::filesystem::create_directory(directoryNameForIx(index));
	};

	int fileCount = static_cast<int>(filenames.size());
	int partitionSize = static_cast<int>(ceil(static_cast<double>(fileCount) / static_cast<double>(numPartitions)));

	int ctr = 0, partitionIx = 1;

	creatDirForIx(partitionIx);

	for(string filename : filenames) {
		if(ctr >= partitionSize) {
			partitionIx++;
			ctr = 0;
			creatDirForIx(partitionIx);
		}
		boost::filesystem::copy(filename, directoryNameForIx(partitionIx) + sep + boost::filesystem::path(filename).filename().string());
		ctr++;
	}
}

vector<int> Utils::deserializeSchedule(int njobs, const string &filename) {
	vector<int> sts(njobs);
	auto lines = Utils::readLines(filename);
	for(auto line : lines) {
		if(!boost::contains(line, "->")) continue;
		int j, stj;
		std::sscanf(line.c_str(), "%d->%d", &j, &stj);
		sts[j-1] = stj;
	}
	return sts;
}

vector<string> Utils::splitLines(const string& s) {
	vector<string> lines;
	split(lines, s, boost::is_any_of("\n"));
	return lines;
}

vector<string> Utils::parseArgumentList(int argc, const char** argv) {
	return Utils::constructVector<string>(argc-1, [&argv](int i) {
		return argv[i+1];
	});
}

bool Utils::fileExists(const std::string &filename) {
	FILE *fp = fopen(filename.c_str(), "r");
	if(fp != nullptr) {
		fclose(fp);
		return true;
	}
	return false;
}

float Utils::average(const std::vector<int> &values) {
	return static_cast<float>(std::accumulate(values.begin(), values.end(), 0)) / static_cast<float>(values.size());
}

float Utils::average(const std::vector<float> &values) {
	return static_cast<float>(std::accumulate(values.begin(), values.end(), 0.0f)) / static_cast<float>(values.size());
}

int Utils::sum(const std::vector<int> &values) {
	int acc = 0;
	for(int i=0; i<values.size(); i++)
		acc += values[i];
	return acc;
}

float Utils::variance(const std::vector<float> &values) {
	if(values.size() <= 1) return 0.0f;
	const float mean = average(values);
	float acc = 0.0f;
	for(float v : values) {
		acc += (v - mean)*(v - mean);
	}
	return acc / static_cast<float>(values.size() - 1);
}

Matrix<char> Utils::transitiveClosure(const Matrix<char> & mx) {
	assert(mx.getM() == mx.getN());
	int n = mx.getN();
	Matrix<char> tc = mx;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			for (int k = 0; k < n; k++)
				if (tc(i, k) && tc(k, j)) tc(i, j) = 1;
	return tc;
}


