//
// Created by André Schnabel on 10.10.17.
//

#pragma once

#include <string>
#include <fstream>
#include "Stopwatch.h"

namespace Utils {

	class Logger {
	public:
		enum class LogLevel {
			INFO = 0,
			WARNING,
			ERROR
		};

		enum class LogMode {
			QUIET = 0,
			MEDIUM,
			VERBOSE
		};

	private:
		std::string logName;
		std::ofstream f;
		LogMode mode;
		static Logger *instance;

	public:
		Logger(const std::string &_logName, LogMode _mode);

		void log(LogLevel level, const std::string &message);

		static Logger *getInstance();
	};

	class Tracer {
	public:
		enum class TraceMode {
			ONLY_COUNT = 0,
			ONLY_INTERVAL,
			BOTH
		};

		explicit Tracer(const std::string &filePrefix = "SolverTrace", TraceMode _traceMode = TraceMode::ONLY_INTERVAL);
		~Tracer();

		void trace(double slvtime, float bks_objval, int nschedules, int nindividuals, bool trunc_secs = false);

		void countTrace(float bks_objval, int nschedules, int nindividuals);
		void intervalTrace(float bks_objval, int nschedules, int nindividuals);

		void setTraceMode(TraceMode _traceMode);

	private:
		std::ofstream f;
		std::chrono::time_point<std::chrono::system_clock> lupdate;
		double last_slvtime;
		Stopwatch sw;
		TraceMode traceMode;
		int lastNumSchedules;
	};

}