//
// Created by André Schnabel on 23.10.15.
//

#include <regex>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cmath>

#include "Utils.h"
#include "Stopwatch.h"

namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

string Utils::slurp(string filename) {
	ifstream fp(filename);
	if(!fp) throw runtime_error("Unable to open file: " + filename);
	string s((istreambuf_iterator<char>(fp)), istreambuf_iterator<char>());
	return s;
}

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
		sw.start();
		lupdate = chrono::system_clock::now();
		last_slvtime = 0.0;
        if(!f.is_open())
            throw runtime_error("Unable to create " + filePrefix + ".txt!");
        f << "slvtime,bks_objval\n";
		trace(0.0, 0.0f);
    }

    Tracer::~Tracer() {
        f.close();
    }

    void Tracer::trace(double slvtime, float bks_objval, bool trunc_secs) {
		double insecs = (slvtime / 1000.0);
		if (trunc_secs) insecs = trunc(insecs);
		f << (boost::format("%.2f") % insecs) << "," << bks_objval << endl;
    }

	void Tracer::intervalTrace(float bks_objval) {
		double slvtime = sw.look();
		double deltat = chrono::duration<double, milli>(chrono::system_clock::now() - lupdate).count();
		if(slvtime < 1000.0 && deltat >= MSECS_BETWEEN_TRACES_SHORT) {
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval);
		} else if(slvtime >= 1000.0 && last_slvtime < 1000.0) {
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval, true);
		} else if(slvtime >= 1000.0 && deltat >= MSECS_BETWEEN_TRACES_LONG) {
			//cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval, true);
		}
		last_slvtime = slvtime;
	}

	string formattedNow() {
		time_t rawtime;
		struct tm* timeinfo;
		char buffer[80];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
		string str(buffer);
		return str;
	}

	Logger *Logger::instance = nullptr;

	Logger::Logger(const string& _logName, LogMode _mode): logName(_logName), f(logName + "Log.txt"), mode(_mode) {
	}

	void Logger::log(LogLevel level, const string& message) {
		if(mode == LogMode::QUIET || (level == LogLevel::INFO && mode == LogMode::MEDIUM)) return;
		string timestr = formattedNow();
		f << "[" << logName << ", " << timestr << "]: " << message << endl;
		f.flush();
		cout << "[" << logName << ", " << timestr << "]: " << message << endl;		
	}

	Logger* Logger::getInstance() {
		if(!instance)
			instance = new Logger("MainLogger", Utils::Logger::LogMode::VERBOSE);
		return instance;
	}
}
