#pragma once
#include <maths.hpp>
#include <limits>
using namespace math;

void throw_computation_error(char const *msg, char const *file, int line);

#define throw_if_not(cond, msg) \
    if (!(cond))                \
        throw_computation_error(msg, __FILE__, __LINE__);

constexpr inline bool is_equal(double left, double right)
{
    return cabs(left - right) < std::numeric_limits<double>::epsilon() * 1000;
}
