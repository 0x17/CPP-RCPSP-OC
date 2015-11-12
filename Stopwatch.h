#pragma once

#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

class Stopwatch {
	TimePoint stime;
public:
	void start();
	double look() const;
    double lookAndReset();
};
