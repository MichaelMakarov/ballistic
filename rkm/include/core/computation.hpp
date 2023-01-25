#pragma once
#include <optimize.hpp>
#include <observation.hpp>
#include <list>
#include <memory>

using namespace math;

/**
 * @brief Мерный интервал
 *
 */
class measuring_interval
{
public:
    class iterator
    {
        observ_iter _iter;
        size_t _offset{};

    public:
        iterator() = default;
        iterator(observ_iter iter, size_t offset) : _iter{iter}, _offset{offset} {}
        iterator(const iterator &) = default;
        iterator &operator=(const iterator &) = default;
        iterator &operator++()
        {
            _offset = ++_offset % _iter->m.size();
            if (_offset == 0)
            {
                ++_iter;
            }
            return *this;
        }
        iterator operator++(int)
        {
            auto tmp = *this;
            iterator::operator++();
            return tmp;
        }
        const measurement_data &measurement() const
        {
            return _iter->m[_offset];
        }
        const observation_seance &seance() const
        {
            return *_iter;
        }
        friend bool operator==(const iterator &left, const iterator &right)
        {
            return left._iter == right._iter && left._offset == right._offset;
        }
        friend bool operator!=(const iterator &left, const iterator &right)
        {
            return left._iter != right._iter || left._offset != right._offset;
        }
    };

private:
    observ_iter _begin, _end;
    size_t _count;
    time_h _tn, _tk;

public:
    measuring_interval(observ_iter begin, observ_iter end);
    measuring_interval(const measuring_interval &) = default;
    measuring_interval &operator=(const measuring_interval &) = default;
    size_t points_count() const { return _count; }
    time_h tn() const { return _tn; }
    time_h tk() const { return _tk; }
    auto begin() const { return iterator(_begin, 0); }
    auto end() const { return iterator(_end, 0); }

    friend auto seance_iterators(const measuring_interval &inter)
    {
        return std::make_pair(inter._begin, inter._end);
    }
};

/**
 * @brief Вращающийся объект
 */
class rotator
{
public:
    /**
     * @brief Время начального поворота
     */
    time_h tn{};
    /**
     * @brief Ось вращения в АСК
     */
    vec3 axis;
    /**
     * @brief Угловая скорость вращения в рад/с
     */
    double vel{};
    /**
     * @brief Вычисление кватерниона поворота на заданный момент времени.
     *
     * @param t
     * @return quaternion
     */
    quaternion operator()(time_h t) const;
};

/**
 * @brief Логировщик промежуточных вычислений для функции оптимизации
 *
 */
template <size_t _size>
struct basic_logger : public optimization_logger<_size>, public std::list<optimization_iteration<_size>>
{
    using std::list<optimization_iteration<_size>>::list;
    void add(optimization_iteration<_size> const &iter) override
    {
        this->push_back(iter);
    }
};

/**
 * @brief Оценённые параметры вращения
 *
 */
struct rotation_info
{
    /**
     * @brief Параметры вращения
     */
    rotator r;
    /**
     * @brief Нормаль
     *
     */
    vec3 n;
};

/**
 * @brief Базовые параметры движения
 *
 */
struct basic_info
{
    /**
     * @brief Параметры движения
     */
    vec6 v;
    /**
     * @brief Параметры промежуточных вычислений
     */
    basic_logger<6> l;
};

struct extbasic_info
{
    vec6 v;
    basic_logger<7> l;
    double s{};
};

/**
 * @brief Расширенные параметры движения
 *
 */
struct extended_info
{
    /**
     * @brief Параметры движения
     */
    vec6 v;
    /**
     * @brief Параметры промежуточных вычислений
     */
    basic_logger<7> l;
    /**
     * @brief Площадь поверхности, м^2
     */
    double s{};
    /**
     * @brief Коэф-т отражения поверхности
     */
    double r{};
};

/**
 * @brief Результаты расчётов
 *
 */
class computational_output
{
public:
    computational_output() = default;
    computational_output(const computational_output &) = default;
    computational_output(computational_output &&) noexcept;
    computational_output &operator=(const computational_output &) = default;
    computational_output &operator=(computational_output &&) noexcept;

private:
    void move(computational_output &other) noexcept;

public:
    /**
     * @brief Опорный ТЛЕ
     */
    std::shared_ptr<orbit_data> refer;
    /**
     * @brief Мерный интервал
     */
    std::shared_ptr<measuring_interval> inter;
    /**
     * @brief Результаты вычислений по модели движения без учёта солнечного давления
     */
    std::shared_ptr<basic_info> basic;
    /**
     * @brief Результаты вычислений по модели движения с учётом солнечного давления и без учёта вращения
     *
     */
    std::shared_ptr<extbasic_info> extbasic;
    /**
     * @brief Результаты вычислений при оценке параметров вращения
     */
    std::shared_ptr<rotation_info> rotation;
    /**
     * @brief Результаты вычислений по модели движения с учётом солнечного давления и вращения
     */
    std::shared_ptr<extended_info> extended;
};