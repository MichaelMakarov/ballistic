#pragma once
#include <ball.hpp>
#include <maths.hpp>
#include <times.hpp>
#include <geometry.hpp>
#include <rotator.hpp>

using vec55 = math::vec<55>;

class motion_model
{
    geopotential _gpt;
    double _sb;
    // std::vector<geometry> const &_geometries;
    // rotator _rotator;

    double check_height(double const v[3], std::time_t t) const;

public:
    /**
     * @brief Интервал допустимых высот
     *
     */
    math::interval<double> heights{100e3, 10000e3};

public:
    motion_model(std::size_t harmonics, double sball);
    // motion_model(std::size_t harmonics, double sball,
    //              std::vector<geometry> const &geometries,
    //              rotator const &rot);
    math::vec6 operator()(math::vec6 const &v, time_t t);
    vec55 operator()(vec55 const &v, time_t t);
};