#include <auxiliaries.hpp>

double func(double x)
{
    return 1 + x * (2 - x);
}

void test_poly()
{
    constexpr size_t count{10};
    constexpr double step{2.0 / count};
    double x[count], y[count];
    for (size_t i{}; i < count; ++i)
    {
        x[i] = step * i;
        y[i] = func(x[i]);
    }
    auto poly = polyfit<2>(x, y, count);
    double left = poly(0);
    double right = poly(2);
    throw_if_not(is_equal(left, 1) && is_equal(right, 1), "Incorrect values.");
}
