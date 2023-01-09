#include <computation.hpp>
#include <algorithm>

/// функции мерного интервала

bool compare(const observation_seance &left, const observation_seance &right)
{
    return left.m.back().t < right.m.back().t;
}

time_h final_interval_time(observ_iter begin, observ_iter end)
{
    auto iter = std::max_element(begin, end, &compare);
    if (iter->m.empty())
    {
        throw std::runtime_error("Сеанс не содержит измерений.");
    }
    return iter->m.back().t;
}

measuring_interval::measuring_interval(observ_iter begin, observ_iter end) : _begin{begin}, _end{end}, _tn{std::numeric_limits<long long>::max()}, _tk{}
{
    _count = 0;
    for (auto iter = begin; iter != end; ++iter)
    {
        _count += iter->m.size();
        for (auto &m : iter->m)
        {
            _tn = std::min(_tn, m.t);
            _tk = std::max(_tk, m.t);
        }
    }
}

quaternion rotator::operator()(time_h t) const
{
    return quaternion(axis, vel * (t - tn));
}

void computational_output::move(computational_output &other) noexcept
{
    refer = std::move(other.refer);
    inter = std::move(other.inter);
    basic = std::move(other.basic);
    extbasic = std::move(other.extbasic);
    rotation = std::move(other.rotation);
    extended = std::move(other.extended);
}

computational_output::computational_output(computational_output &&other) noexcept
{
    move(other);
}

computational_output &computational_output::operator=(computational_output &&other) noexcept
{
    move(other);
    return *this;
}