#pragma once

#include <chrono>

const double MSECS_BETWEEN_TRACES = 1000.0;

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

class Stopwatch {
	TimePoint stime;
public:
	void start();
	double look() const;
    double lookAndReset();
};
