#pragma once
#include <ball.hpp>
#include <maths.hpp>
#include <timeutility.hpp>
#include <geometry.hpp>
#include <rotator.hpp>

class motion_model
{
    geopotential _gpt;
    double _sball;
    double _scoef;
    std::vector<geometry> const &_geometries;
    rotator _rotator;

public:
    /**
     * @brief Интервал допустимых высот
     *
     */
    math::interval<double> heights{100e3, 10000e3};

public:
    motion_model(std::size_t harmonics, double sball, double scoef,
                 std::vector<geometry> const &geometries,
                 rotator const &rot);
    math::vec6 operator()(math::vec6 const &v, time_t t);
};