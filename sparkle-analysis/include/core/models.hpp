#pragma once
#include <ball.hpp>
#include <maths.hpp>

using time_t = int64_t;

class motion_model
{
    geopotential _gpt;

public:
    math::interval<double> heights{1e5, 1e8};

public:
    motion_model(size_t harmonics);
    math::vec6 operator()(const math::vec6 &v, time_t t);
};
