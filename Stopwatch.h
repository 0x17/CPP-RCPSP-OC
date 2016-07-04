#pragma once

#include <chrono>

const double MSECS_BETWEEN_TRACES_SHORT = 0.01 * 1000.0; // before 1.0secs
const double MSECS_BETWEEN_TRACES_LONG = 1.00 * 1000.0; // after 1.0secs

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

class Stopwatch {
	TimePoint stime;
public:
	void start();
	double look() const;
    double lookAndReset();
};

