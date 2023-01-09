#include <optimize.hpp>
#include <string>
#include <queue>
#include <future>
#include <vector>
#include <memory>
#include <atomic>

void throw_if_index_out_of_range(size_t i, size_t supr)
{
	supr -= 1;
	if (i > supr)
	{
		std::string msg = "Инлекс вне диапазона (" + std::to_string(i) + " > " + std::to_string(supr) + ").";
		throw std::invalid_argument(msg);
	}
}

bool is_equal(double oldval, double newval, double eps)
{
	return std::abs(1 - newval / oldval) < eps;
}

bool is_equal_or_greater(double oldval, double newval, double eps)
{
	return oldval < newval || is_equal(oldval, newval, eps);
}
