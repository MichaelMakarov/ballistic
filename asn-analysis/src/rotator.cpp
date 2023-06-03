#include <rotator.hpp>
#include <algorithm>
#include <stdexcept>

rotator::rotator(std::vector<rotation_measurement>::const_iterator begin,
                 std::vector<rotation_measurement>::const_iterator end)
    : _begin{begin}, _end{end}, _count{static_cast<std::size_t>(std::distance(begin, end))}
{
    if (_count < 2)
    {
        throw std::invalid_argument("There must be at least 2 rotation measurements.");
    }
    _delta = ((end - 1)->t - begin->t) / (_count - 1);
}

math::quaternion rotator::get_quaternion(time_t tt) const
{
    auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(tt));
    math::quaternion q;
    if (t >= _begin->t && t <= _end->t)
    {
        std::size_t index = (t - _begin->t) / _delta;
        index = std::min(index, _count - 2);
        auto left = _begin + index;
        auto right = left + 1;
        double delta = static_cast<double>(_delta.count());
        double dt = static_cast<double>((t - left->t).count());
        double mult = dt / delta;
        q = left->q * (1 - mult) + right->q * mult;
    }
    else
    {
        throw std::invalid_argument("Time input is out of bounds the rotator has.");
    }
    return q;
}
