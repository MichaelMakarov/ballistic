#pragma once
#include <chrono>

namespace times {
	/// <summary>
	/// Interface to get time interval
	/// </summary>
	class stopwatch {
		std::chrono::steady_clock::time_point _start, _finish;
	public:
		/// <summary>
		/// Starts time measurement.
		/// </summary>
		void start();
		/// <summary>
		/// Finishes time measurement.
		/// </summary>
		void finish();
		/// <summary>
		/// Returns measured interval duration.
		/// </summary>
		/// <returns></returns>
		double duration();
	};
}