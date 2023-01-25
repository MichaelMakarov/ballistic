#include <optimize.hpp>

namespace math
{

	bool is_equal(double oldval, double newval, double eps)
	{
		if (std::abs(newval - oldval) < eps)
			return true;
		return std::abs(1 - newval / oldval) < eps;
	}

	double residual_function(vector const &v)
	{
		return v * v;
	}

}