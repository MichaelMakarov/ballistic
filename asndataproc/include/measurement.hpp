#pragma once
#include <timeutility.hpp>
#include <maths.hpp>

/**
 * @brief Измерение движения
 *
 */
struct motion_measurement
{
    time_point_t t;
    double v[6];
};

/**
 * @brief Измерение вращения
 *
 */
struct rotation_measurement
{
    time_point_t t;
    math::quaternion q;
};