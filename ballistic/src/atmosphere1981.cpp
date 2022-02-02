#include "atmosphere1981.h"
#include <cmath>

namespace ball {
	constexpr double height[9] = {
		0.0,			0.2e+5,			 0.6e+5,
		1.0e+5,			1.5e+5,			 3.0e+5,
		6.0e+5,			9.0e+5,			 10e6
	};
	constexpr double a0[9] = {
		0.12522,		0.91907e-2,		 0.31655e-4,
		0.54733e-7,		0.20474e-9,		 0.19019e-11,
		0.11495e-13,	0.58038e-15,	 1e-17
	};
	constexpr double k1[9] = {
		-0.20452e-8,	0.62669e-9,		-0.86999e-9,
		0.12870e-8,		0.10167e-9,		 0.97266e-11,
		0.15127e-10,	0.0,			 0
	};
	constexpr double k2[9] = {
		0.90764e-4,		0.16739e-3,		 0.12378e-3,
		0.17527e-3,		0.45825e-4,		 0.19885e-4,
		0.14474e-4,		0.39247e-5,		 1e-6
	};

	double atmosphere1981::density(double h) const {
		size_t i{ 1 };
		for (; i < 9 && height[i] < h; ++i);
		h -= height[--i];
		return a0[i] * std::exp(h * (h * k1[i] - k2[i]));
	}
}