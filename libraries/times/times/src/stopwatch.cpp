#include <stopwatch.h>

namespace times {
	void stopwatch::start() {
		_finish = _start = std::chrono::steady_clock::now();
	}

	void stopwatch::finish() {
		_finish = std::chrono::steady_clock::now();
	}

	double stopwatch::duration() {
		return std::chrono::duration<double, std::nano>(_finish - _start).count() * 1e-9;
	}
}