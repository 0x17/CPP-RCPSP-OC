//
// Created by André Schnabel on 23.10.15.
//

#include <regex>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Utils.h"

namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

vector<string> Utils::readLines(string filename) {
    vector<string> lines;
    string line;
    ifstream f(filename);
    if(!f) throw runtime_error("Unable to open file: " + filename);
    while(!f.eof()) {
        getline(f, line);
        lines.push_back(line);
    }
    return lines;
}

int Utils::extractIntFromStr(string s, string rx) {
    regex rxObj(rx);
    smatch result;
    regex_search(s, result, rxObj);
    return stoi(result[1]);
}

vector<int> Utils::extractIntsFromLine(string line) {
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

void Utils::serializeSchedule(vector<int> & sts, const string filename) {
	ofstream f(filename);
	if(f.is_open()) {
		for (int j = 0; j < sts.size(); j++) {
			f << (j + 1) << "->" << sts[j];
			if (j < sts.size() - 1) f << "\n";
		}
		f.close();
	}
	
}

void Utils::serializeProfit(float profit, const string filename) {
	spit(to_string(profit), filename);
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

void Utils::spit(const string s, const string filename) {
    ofstream f(filename);
    if(f.is_open()) {
        f << s;
        f.close();
    }
}

list<string> Utils::filenamesInDirWithExt(const string dir, const string ext) {
	list<string> fnames;
	fs::path p(dir);
	for(auto it = fs::directory_iterator(p); it != fs::directory_iterator(); ++it) {
        auto entry = *it;
        string filename = entry.path().string();
        if(fs::is_regular_file(entry) && algo::ends_with(filename, ext))
			fnames.push_back(filename);

	}
	return fnames;
}

void Utils::spitAppend(const string s, const string filename) {
    ofstream f(filename, ios_base::app);
    if(f.is_open()) {
        f << s;
        f.close();
    }
}

namespace Utils {
    Tracer::Tracer(const string filePrefix) : f(filePrefix + ".txt") {
        if(!f.is_open())
            throw runtime_error("Unable to create " + filePrefix + ".txt!");
        f << "slvtime,bks_objval\n";
		trace(0.0, 0.0f);
    }

    Tracer::~Tracer() {
        f.close();
    }

    void Tracer::trace(double slvtime, float bks_objval) {
		f << (boost::format("%.2f") % (slvtime / 1000.0)) << "," << bks_objval << endl;
    }
}