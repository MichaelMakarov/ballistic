#pragma once

#include <measurement.hpp>
#include <vector>

class rotator
{
    std::vector<rotation_measurement> const &_measurements;
    time_point_t::duration _delta;

public:
    rotator(std::vector<rotation_measurement> const &measurements);
    math::quaternion get_quaternion(time_point_t t) const;
};