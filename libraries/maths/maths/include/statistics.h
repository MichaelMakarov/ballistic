#pragma once
#include <cmath>

namespace math {

	/// <summary>
	/// Computes mean value of the list of values.
	/// </summary>
	/// <param name="begin">begin position</param>
	/// <param name="end">end position</param>
	/// <returns>mean value</returns>
	template<typename forw_it>
	constexpr double mean(forw_it begin, forw_it end) {
		double result{};
		size_t count;
		for (count = 0; begin != end; ++begin, ++count) {
			result += *begin;
		}
		return result / count;
	}
	/// <summary>
	/// Computes standard deviation of the list of values.
	/// </summary>
	/// <param name="begin">begin position</param>
	/// <param name="end">end position</param>
	/// <returns>standard deviation</returns>
	template<typename forw_it>
	constexpr double std(forw_it begin, forw_it end) {
		double result{};
		size_t count;
		for (count = 0; begin != end; ++begin, ++count) {
			result += (*begin) * (*begin);
		}
		return std::sqrt(result / count);
	}

}