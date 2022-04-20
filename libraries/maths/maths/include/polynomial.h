#pragma once
#include <cstddef>

namespace math {

	/// <summary>
	/// Power polynomial
	/// </summary>
	template<size_t _degree> 
	struct polynomial {
		constexpr size_t degree() const noexcept { return _degree; }
		constexpr double* data() noexcept { return _elems; }
		constexpr const double* data() const noexcept { return _elems; }

		constexpr double& operator[] (size_t index) { return _elems[index]; }
		constexpr const double& operator[] (size_t index) const { return _elems[index]; }

		constexpr double operator() (double x) const {
			double mult{ x }, total{ _elems[0] };
			for (size_t i{ 1 }; i <= _degree; ++i) {
				total += mult * _elems[i];
				mult *= x;
			}
			return total;
		}
	public:
		double _elems[_degree + 1]{};
	};
}