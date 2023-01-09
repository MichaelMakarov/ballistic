#include <rotation.hpp>
#include <optimize.hpp>
#include <forecast.hpp>
#include <transform.hpp>
#include <ball.hpp>
#include <algorithm>
#include <array>
#include <cmath>

class interval_range
{
public:
    double min;
    double max;
    size_t count;

    double step() const
    {
        return (max - min) / count;
    }

    class iterator
    {
        const interval_range *_base;
        size_t _index;

    public:
        iterator(const interval_range &base, size_t index) : _base{&base}, _index{index} {}
        iterator(const iterator &) noexcept = default;
        iterator &operator=(const iterator &) noexcept = default;
        iterator &operator++()
        {
            ++_index;
            return *this;
        }
        double operator*() const
        {
            return _base->min + _base->step() * _index;
        }
        bool operator!=(const iterator &other)
        {
            return _index != other._index || _base != other._base;
        }
    };

    auto begin() const { return iterator(*this, 0); }
    auto end() const { return iterator(*this, count + 1); }
};

struct square_info
{
    /**
     * @brief Площадь поверхности, м^2
     *
     */
    double square;
    /**
     * @brief Отношения площадей к начальному значению
     *
     */
    std::vector<double> ratios;

    square_info() = default;
    square_info(square_info &&other) noexcept : square{other.square}, ratios{std::move(other.ratios)}
    {
    }
    square_info &operator=(square_info &&other) noexcept
    {
        std::swap(square, other.square);
        ratios.swap(other.ratios);
        return *this;
    }
};

struct surface_normal : vec3
{
    surface_normal(double incl, double asc)
    {
        const double sph[3]{1, incl, asc};
        transform<abs_cs, sph_cs, abs_cs, ort_cs>::forward(sph, data());
    }
};

struct solar_unit : vec3
{
    solar_unit(time_h t)
    {
        solar_model::coordinates(t, data());
        normalize();
    }
};

double sqr_distance(const double *left, const double *right)
{
    double res{};
    for (size_t i{}; i < 3; ++i)
    {
        res += sqr(left[i] - right[i]);
    }
    return res;
}

double cos_phase_angle(const double *o, const double *v, time_h t)
{
    vec3 obs{o[0], o[1], o[2]};
    vec3 sat{v[0], v[1], v[2]};
    double buf[3]{};
    solar_model::coordinates(t, buf);
    vec3 sun;
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, sidereal_time_mean(t), sun.data());
    auto angle = cos_angle_of(sun - sat, obs - sat);
    return angle;
}

/**
 * @brief Нахождение площади отражающей поверхности, умноженной на отражаюший к-т, в проекции на ортогональную к направлению на Солнце плоскость.
 *
 * @param magn звёздная величина
 * @param phase фазовая функция (фазовый угол - между направлением на Солнце и направлением на наблюдателя)
 * @param rsqr квадрат расстояния между объектом и наблюдателем
 * @return площадь поверхности в м^2
 */
double square_from_magnitude(double magn, double cos_phase, double rsqr)
{
    // фазовая функция F(phi) = (cos(phi / 2))^2 / pi, где cos(phi / 2) = sqrt( (1 + cos(phi)) / 2 )
    auto phase_func = (cos_phase + 1) * (0.5 / pi);
    return std::pow(10.0, (magn + 26.58) * (1 / -2.5)) * rsqr / phase_func;
}
/**
 * @brief Нахождение площади отражающей поверхности в проекции на ортогональную в направлению на Солнце плоскость.
 *
 * @param obs координаты наблюдателя в ГСК
 * @param vc координаты объекта в ГСК
 * @param magn звёздная величина
 * @param t время
 * @return double
 */
double reflective_square(const double *obs, const double *vc, double magn, time_h t)
{
    double phase = cos_phase_angle(obs, vc, t);
    double rsqr = sqr_distance(vc, obs);
    double sq = square_from_magnitude(magn, phase, rsqr);
    return sq;
}

auto compute_surface(forecast const &f, const measuring_interval &inter)
{
    square_info info{};
    info.ratios.resize(inter.points_count());
    auto begin = std::begin(inter);
    auto end = std::end(inter);
    for (size_t i{}; begin != end; ++begin, ++i)
    {
        auto &meas = begin.measurement();
        auto mp = f.point(meas.t);
        auto sq = reflective_square(begin.seance().o, mp.v.data(), meas.m, meas.t);
        info.ratios[i] = sq;
    }
    info.square = info.ratios.front();
    auto mult = 1 / info.square;
    for (auto &elem : info.ratios)
        elem *= mult;
    info.ratios.erase(std::begin(info.ratios));
    return info;
}

auto compute_surface(const measuring_interval &inter, const vec<5> &params)
{
    rotator rp{.tn = inter.tn(), .axis = surface_normal{params[3], params[4]}, .vel = params[2]};
    surface_normal norm{params[0], params[1]};
    std::vector<double> resid(inter.points_count() - 1);
    auto begin = std::begin(inter);
    auto end = std::end(inter);
    // вектор направления на Солнце
    solar_unit sun0{begin.measurement().t};
    // косинус угла между нормалью к поверхности и направлением на Солнце
    double cos0 = sun0 * norm;
    ++begin;
    for (size_t i{}; begin != end; ++begin, ++i)
    {
        auto &meas = begin.measurement();
        auto n = rp(meas.t).rotate(norm);
        solar_unit sun{meas.t};
        double cos = n * sun;
        cos /= cos0;
        cos *= sign(cos);
        resid[i] = cos;
    }
    return resid;
}

/**
 * @brief Приближение массива данных функцией синуса y(t) = a * sin(f * t + dx) + dy
 *
 * @param arr_x
 * @param arr_y
 * @return double
 */
double estimate_frequency(const std::vector<double> &arr_x, const std::vector<double> &arr_y)
{
    auto minmax_iter = std::minmax_element(std::begin(arr_y), std::end(arr_y));
    double dy = *minmax_iter.first;
    double a = *minmax_iter.second - dy;
    a *= 1.01;
    std::vector<double> buf(arr_x.size());
    // заполняем по горизонтали и решаем задачу МНК f * t + dx = asin( (y(t) - dy) / a )
    for (size_t i{}; i < buf.size(); ++i)
    {
        buf[i] = std::asin((arr_y[i] - dy) / a);
    }
    auto poly = polyfit<1>(std::begin(arr_x), std::begin(buf), buf.size());
    return poly[1];
}

double estimate_angular_velocity(const measuring_interval &inter, const std::vector<double> &ratios)
{
    if (ratios.empty())
        return {};
    std::vector<double> times;
    times.reserve(ratios.size());
    auto tn = std::begin(inter).measurement().t;
    for (auto iter = std::begin(inter); ++iter != std::end(inter);)
    {
        times.push_back(iter.measurement().t - tn);
    }
    auto freq = estimate_frequency(times, ratios);
    return freq;
}

double compute_residual(const vec<5> &params, const measuring_interval &inter, const std::vector<double> &ref)
{
    auto arr = compute_surface(inter, params);
    double resid{};
    for (size_t i{}; i < arr.size(); ++i)
    {
        resid += sqr(arr[i] - ref[i]);
    }
    return resid;
}

auto select(const std::array<interval_range, 5> &rs, const std::vector<double> &ref, const measuring_interval &inter, vec<5> &output)
{
    double min_resid = std::numeric_limits<double>::max();
    for (auto i : rs[0])
    {
        for (auto a : rs[1])
        {
            for (auto w : rs[2])
            {
                for (auto ni : rs[3])
                {
                    for (auto na : rs[4])
                    {
                        const vec<5> temp{i, a, w, ni, na};
                        auto resid = compute_residual(temp, inter, ref);
                        if (resid < min_resid)
                        {
                            min_resid = resid;
                            output = temp;
                        }
                    }
                }
            }
        }
    }
    return min_resid;
}

vec<5> estimate_rotation(const std::vector<double> &ratios, const measuring_interval &inter)
{
    constexpr size_t grid_size{10};
    // оцениваем частоту изменения данных площади поверхности
    double w = estimate_angular_velocity(inter, ratios);
    // сетка параметров
    std::array<interval_range, 5> ranges;
    // заполняем изначальные диапазоны сетки
    ranges[0] = ranges[3] = interval_range{.min = -pi, .max = pi, .count = grid_size};
    ranges[1] = ranges[4] = interval_range{.min = -pi / 2, .max = pi / 2, .count = grid_size};
    ranges[2] = interval_range{.min = 5e-1 * w, .max = 25e-1 * w, .count = grid_size};
    // параметры вращения
    vec<5> rp;

    double prev_resid{1}, next_resid{};
    while (!is_equal(prev_resid, next_resid, 1e-2))
    {
        prev_resid = next_resid;
        next_resid = select(ranges, ratios, inter, rp);
        // обновляем диапазоны сетки
        for (size_t i{}; i < ranges.size(); ++i)
        {
            auto &r = ranges[i];
            auto step = r.step();
            r.min = rp[i] - step;
            r.max = rp[i] + step;
        }
    }
    return rp;
}

void estimate_rotation(measuring_interval const &inter, double const (&v)[6], time_h tn, rotation_info &info)
{
    // параметры движения
    motion_params mp;
    mp.t = tn;
    std::copy(v, v + 6, mp.v.data());
    auto f = make_forecast(mp, inter.tk());
    auto surf_info = compute_surface(f, inter);
    // параметры вращения
    auto r = estimate_rotation(surf_info.ratios, inter);
    info.n = surface_normal{r[3], r[4]};
    info.r = rotator{.tn = inter.tn(), .axis = surface_normal(r[0], r[1]), .vel = r[2]};
}
