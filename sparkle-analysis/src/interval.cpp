#include <interval.hpp>
#include <algorithm>
#include <numeric>

measuring_iterator &measuring_iterator::operator++()
{
    _offset = ++_offset % _iter->m.size();
    if (_offset == 0)
    {
        ++_iter;
    }
    return *this;
}

measuring_iterator measuring_iterator::operator++(int)
{
    auto tmp = *this;
    operator++();
    return tmp;
}

measurement_data const &measuring_iterator::measurement() const
{
    if (_iter->m.empty())
    {
        throw std::runtime_error("Attempt to get a measurement from an empty seance.");
    }
    return _iter->m[_offset];
}

observation_seance const &measuring_iterator::seance() const
{
    return *_iter;
}

bool operator==(const measuring_iterator &left, const measuring_iterator &right)
{
    return left._iter == right._iter && left._offset == right._offset;
}

bool operator!=(const measuring_iterator &left, const measuring_iterator &right)
{
    return left._iter != right._iter || left._offset != right._offset;
}

/// функции мерного интервала

bool compare(const observation_seance &left, const observation_seance &right)
{
    return left.m.back().t < right.m.back().t;
}

std::size_t measuring_interval::points_count() const
{
    return std::accumulate(_begin, _end, std::size_t{}, [](std::size_t n, observation_seance const &s)
                           { return n + s.m.size(); });
}

void check_seances_range(observ_iter begin, observ_iter end)
{
    if (std::distance(begin, end) == 0)
    {
        throw std::runtime_error("An empty range of observation seances.");
    }
}

time_type measuring_interval::tn() const
{
    check_seances_range(_begin, _end);
    auto t = time_type::max();
    for (auto iter = _begin; iter != _end; ++iter)
    {
        for (auto &m : iter->m)
        {
            t = std::min(t, m.t);
        }
    }
    return t;
}

time_type measuring_interval::tk() const
{
    check_seances_range(_begin, _end);
    auto t = time_type::min();
    for (auto iter = _begin; iter != _end; ++iter)
    {
        for (auto &m : iter->m)
        {
            t = std::max(t, m.t);
        }
    }
    return t;
}