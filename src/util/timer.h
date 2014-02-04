#pragma once

#include <sys/time.h>

namespace Util {
class Timer {
private:
	double start;
	double paused;
public:

	Timer() : start(Timestamp()), paused(0) {}

	void restart() {
		start = Timestamp(); 
		paused = 0;
	}

	double elapsedMilliseconds() {
		double current = Timestamp();
		return current - start;
	}

	void pause() {
		paused = Timestamp();
	}

	void resume() {
		double current = Timestamp();
		start += current - paused;
		paused = 0;
	}

private:
	/** Returns a timestamp ('now') in miliseconds (incl. a fractional part). */
	inline double Timestamp() {
		timeval tp;
		gettimeofday(&tp, NULL);
		double ms = double(tp.tv_usec) / 1000.;
		return tp.tv_sec*1000 + ms;
	}
};
}
