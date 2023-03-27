#include <timer.hpp>
#include <chrono>

class watchtimer_impl
{
    std::chrono::high_resolution_clock::time_point _tn;

public:
    watchtimer_impl()
    {
        _tn = std::chrono::high_resolution_clock::now();
    }
    auto get_duration() const
    {
        auto tk = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(tk - _tn);
    }
};

void swap(watchtimer_impl *&left, watchtimer_impl *&right) noexcept
{
    auto temp = left;
    left = right;
    right = temp;
}

watchtimer::watchtimer() : _impl{new watchtimer_impl}
{
}

watchtimer::watchtimer(watchtimer const &other) : watchtimer()
{
    *_impl = *other._impl;
}

watchtimer::watchtimer(watchtimer &&other) noexcept : _impl{nullptr}
{
    swap(_impl, other._impl);
}

watchtimer::~watchtimer()
{
    if (_impl)
    {
        delete _impl;
    }
}
watchtimer &watchtimer::operator=(watchtimer const &other)
{
    *_impl = *other._impl;
    return *this;
}

watchtimer &watchtimer::operator=(watchtimer &&other) noexcept
{
    swap(_impl, other._impl);
    return *this;
}

std::ostream &operator<<(std::ostream &os, watchtimer const &t)
{
    return os << t._impl->get_duration();
}