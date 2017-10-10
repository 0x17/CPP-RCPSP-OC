//
// Created by André Schnabel on 10.10.17.
//

#include <iostream>
#include <cmath>

#include "Logger.h"
#include "Utils.h"

using namespace std;

namespace Utils {

	Logger *Logger::instance = nullptr;

	Logger::Logger(const string &_logName, LogMode _mode) : logName(_logName), f(logName + "Log.txt"), mode(_mode) {
	}

	void Logger::log(LogLevel level, const string &message) {
		if (mode == LogMode::QUIET || (level == LogLevel::INFO && mode == LogMode::MEDIUM)) return;
		string timestr = formattedNow();
		f << "[" << logName << ", " << timestr << "]: " << message << std::endl;
		f.flush();
		std::cout << "[" << logName << ", " << timestr << "]: " << message << std::endl;
	}

	Logger *Logger::getInstance() {
		if (!instance)
			instance = new Logger("MainLogger", Utils::Logger::LogMode::VERBOSE);
		return instance;
	}

	Tracer::Tracer(const string &filePrefix) : f(filePrefix + ".txt") {
		sw.start();
		lupdate = std::chrono::system_clock::now();
		last_slvtime = 0.0;
		if(!f.is_open())
			throw std::runtime_error("Unable to create " + filePrefix + ".txt!");
		f << "slvtime,bks_objval\n";
		trace(0.0, 0.0f);
	}

	Tracer::~Tracer() {
		f.close();
	}

	void Tracer::trace(double slvtime, float bks_objval, bool trunc_secs) {
		double insecs = (slvtime / 1000.0);
		if (trunc_secs) insecs = trunc(insecs);
		// FIXME: Specify number of decimal places!
		f << (boost::format("%.2f") % insecs) << "," << bks_objval << std::endl;
	}

	void Tracer::intervalTrace(float bks_objval) {
		double slvtime = sw.look();
		double deltat = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - lupdate).count();
		if(slvtime < 1000.0 && deltat >= MSECS_BETWEEN_TRACES_SHORT) {
			lupdate = std::chrono::system_clock::now();
			trace(slvtime, bks_objval);
		} else if(slvtime >= 1000.0 && last_slvtime < 1000.0) {
			lupdate = std::chrono::system_clock::now();
			trace(slvtime, bks_objval, true);
		} else if(slvtime >= 1000.0 && deltat >= MSECS_BETWEEN_TRACES_LONG) {
			//cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
			lupdate = std::chrono::system_clock::now();
			trace(slvtime, bks_objval, true);
		}
		last_slvtime = slvtime;
	}

}