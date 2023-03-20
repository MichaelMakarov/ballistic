#pragma once
#include <ball.hpp>
#include <maths.hpp>

class motion_model
{
    geopotential _gpt;
    double _sball;
    double _scoef;

public:
    /**
     * @brief Интервал допустимых высот
     *
     */
    math::interval<double> heights{100e3, 10000e3};

public:
    motion_model(std::size_t harmonics, double sball, double scoef);
    math::vec6 operator()(math::vec6 const &v, time_type t);
};