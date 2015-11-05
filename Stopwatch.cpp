#include "Stopwatch.h"

using namespace std::chrono;

void Stopwatch::start() {
	stime = system_clock::now();
}

double Stopwatch::look() const {
	auto diff = system_clock::now() - stime;
	return duration<double, std::milli>(diff).count();
}

double Stopwatch::lookAndReset() {
    double state = look();
    start();
    return state;
}
