#pragma once
#include <times.hpp>

/**
 * @brief Измерение движения
 *
 */
struct motion_measurement
{
    time_type t;
    double v[6];
};

/**
 * @brief Измерение вращения
 *
 */
struct rotation_measurement
{
    time_type t;
    double q[4];
};