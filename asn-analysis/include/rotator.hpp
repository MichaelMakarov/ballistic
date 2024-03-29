#pragma once

#include <measurement.hpp>
#include <vector>

class rotator
{
    std::vector<rotation_measurement>::const_iterator _begin, _end;
    time_type::duration _delta;
    std::size_t _count;

public:
    rotator(std::vector<rotation_measurement>::const_iterator begin,
            std::vector<rotation_measurement>::const_iterator end);
    math::quaternion get_quaternion(time_t t) const;
};