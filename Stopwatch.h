#pragma once

#include <chrono>

class Stopwatch {
	std::chrono::time_point<std::chrono::system_clock> stime;
public:
	void start();
	double look();
    double lookAndReset();
};
