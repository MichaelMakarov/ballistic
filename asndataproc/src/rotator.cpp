#include <rotator.hpp>
#include <formatting.hpp>
#include <algorithm>

rotator::rotator(std::vector<rotation_measurement> const &measurements)
    : _measurements{measurements}
{
    if (measurements.size() < 2)
    {
        throw_invalid_argument("There must be at least 2 rotation measurements.");
    }
    _delta = (measurements.back().t - measurements.front().t) / (measurements.size() - 1);
}

math::quaternion rotator::get_quaternion(time_point_t t) const
{
    math::quaternion q;
    auto &first_m = _measurements.front();
    if (t >= first_m.t && t <= _measurements.back().t)
    {

        std::size_t index = (t - first_m.t) / _delta;
        index = std::min(index, _measurements.size() - 2);
        auto left = _measurements[index];
        auto right = _measurements[index + 1];
        double delta = static_cast<double>(_delta.count());
        double dt = static_cast<double>((t - left.t).count());
        q = left.q * delta + (right.q - left.q) * dt;
    }
    else
    {
        throw_invalid_argument("Time input is out of bounds the rotator has.");
    }
    return q;
}
