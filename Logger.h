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
		std::ofstream f;
		std::chrono::time_point<std::chrono::system_clock> lupdate;
		double last_slvtime;
		Stopwatch sw;
	public:
		explicit Tracer(const std::string &filePrefix = "SolverTrace");
		~Tracer();
		void trace(double slvtime, float bks_objval, bool trunc_secs = false);
		void intervalTrace(float bks_objval);
	};

}