#pragma once
#include <observation.hpp>
#include <maths.hpp>

class rotator
{
public:
    /**
     * @brief Время начального поворота
     */
    time_t tn{};
    /**
     * @brief Ось вращения в АСК
     */
    math::vec3 ax;
    /**
     * @brief Угловая скорость вращения в рад/с
     */
    double w{};
    /**
     * @brief Вычисление кватерниона поворота на заданный момент времени.
     *
     * @param t
     * @return quaternion
     */
    math::quaternion operator()(time_t t) const
    {
        return math::quaternion(ax, w * (t - tn));
    }
};

using observ_iter = std::vector<observation_seance>::const_iterator;

class measuring_iterator
{
    observ_iter _iter;
    std::size_t _offset;

public:
    measuring_iterator(observ_iter iter, std::size_t offset) : _iter{iter}, _offset{offset} {}
    measuring_iterator(measuring_iterator const &) = default;
    measuring_iterator &operator=(measuring_iterator const &) = default;
    measuring_iterator &operator++();
    measuring_iterator operator++(int);
    measurement_data const &measurement() const;
    observation_seance const &seance() const;
    friend bool operator==(measuring_iterator const &, measuring_iterator const &);
    friend bool operator!=(measuring_iterator const &, measuring_iterator const &);
};

/**
 * @brief Мерный интервал
 *
 */
class measuring_interval
{
    observ_iter _begin, _end;

public:
    measuring_interval(observ_iter begin, observ_iter end) : _begin{begin}, _end{end} {}
    measuring_interval(const measuring_interval &) = default;
    measuring_interval &operator=(const measuring_interval &) = default;
    std::size_t points_count() const;
    time_type tn() const;
    time_type tk() const;
    auto begin() const { return measuring_iterator(_begin, 0); }
    auto end() const { return measuring_iterator(_end, 0); }
};
