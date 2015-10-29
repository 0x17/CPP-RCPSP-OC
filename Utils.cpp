//
// Created by André Schnabel on 23.10.15.
//

#include <regex>
#include <fstream>
#include "Utils.h"

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
			f << (j + 1) << "->" << (sts[j] + 1);
			if (j < sts.size() - 1) f << "\n";
		}
		f.close();
	}
	
}