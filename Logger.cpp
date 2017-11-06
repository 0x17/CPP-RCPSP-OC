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
		f << "[" << logName << ", " << timestr << "]: " << message << endl;
		f.flush();
		cout << "[" << logName << ", " << timestr << "]: " << message << endl;
	}

	Logger *Logger::getInstance() {
		if (!instance)
			instance = new Logger("MainLogger", Utils::Logger::LogMode::VERBOSE);
		return instance;
	}

	Tracer::Tracer(const string &filePrefix, TraceMode _traceMode) : f(filePrefix + ".txt"), traceMode(_traceMode), lastNumSchedules(0) {
		sw.start();
		lupdate = chrono::system_clock::now();
		last_slvtime = 0.0;
		if(!f.is_open())
			throw runtime_error("Unable to create " + filePrefix + ".txt!");
		f << "slvtime,bks_objval,nschedules,nindividuals\n";
		trace(0.0, 0.0f, 0, 0);
	}

	Tracer::~Tracer() {
		f.close();
	}

	void Tracer::trace(double slvtime, float bks_objval, int nschedules, int nindividuals, bool trunc_secs) {
		double insecs = (slvtime / 1000.0);
		if (trunc_secs) insecs = trunc(insecs);
		// FIXME: Specify number of decimal places!
		f << (boost::format("%.2f") % insecs) << "," << bks_objval << "," << nschedules << "," << nindividuals << endl;
	}

#define FIRST_EXCEED(curVal, lastVal, threshold) curVal >= threshold && lastVal < threshold

	void Tracer::countTrace(float bks_objval, int nschedules, int nindividuals) {		
		if(traceMode == TraceMode::ONLY_INTERVAL) return;

		if(FIRST_EXCEED(nschedules, lastNumSchedules, 1000)
			|| FIRST_EXCEED(nschedules, lastNumSchedules, 5000)
			|| FIRST_EXCEED(nschedules, lastNumSchedules, 50000)) {
			trace(sw.look(), bks_objval, nschedules, nindividuals);
		}
		lastNumSchedules = nschedules;
	}

	void Tracer::intervalTrace(float bks_objval, int nschedules, int nindividuals) {
		if (traceMode == TraceMode::ONLY_COUNT) return;
		double slvtime = sw.look();
		double deltat = chrono::duration<double, milli>(chrono::system_clock::now() - lupdate).count();
		if(slvtime < 1000.0 && deltat >= MSECS_BETWEEN_TRACES_SHORT) {
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval, nschedules, nindividuals);
		} else if(slvtime >= 1000.0 && last_slvtime < 1000.0) {
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval, nschedules, nindividuals, true);
		} else if(slvtime >= 1000.0 && deltat >= MSECS_BETWEEN_TRACES_LONG) {
			//cout << "Nodes visited = " << nodeCtr << ", Boundings = " << boundCtr << ", Opt = " << lb << ", Time = " << (boost::format("%.2f") % (sw.look() / 1000.0)) << endl;
			lupdate = chrono::system_clock::now();
			trace(slvtime, bks_objval, nschedules, nindividuals, true);
		}
		last_slvtime = slvtime;
	}

	void Tracer::setTraceMode(TraceMode _traceMode) {
		traceMode = _traceMode;
	}

}
