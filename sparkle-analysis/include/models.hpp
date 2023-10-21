#pragma once
#include <ball.hpp>
#include <maths.hpp>

using time_t = int64_t;
using vec42 = math::vec<42>;
using vec55 = math::vec<55>;

class motion_model
{
    geopotential _gpt;
    double _s;

    void verify_height(double const v[3], time_t t);

public:
    math::interval<double> heights{1e5, 1e8};

public:
    motion_model(size_t harmonics, double s);
    math::vec6 operator()(const math::vec6 &v, time_t t);
    vec55 operator()(vec55 const &v, time_t t);
};