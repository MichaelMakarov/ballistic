#include <config.hpp>
#include <fileutils.hpp>
#include <printutility.hpp>
#include <measurement.hpp>
#include <figure.hpp>
#include <forecast.hpp>
#include <rotator.hpp>
#include <optimization.hpp>
#include <algorithm>
#include <iomanip>
#include <cstdarg>

constexpr std::size_t vecsize{7};

class computation_logger : public math::iterations_saver, public std::vector<math::iteration>
{
public:
    using std::vector<math::iteration>::vector;
    void save(math::iteration &&iter) override
    {
        push_back(std::move(iter));
    }
};

class model_measurer : public math::residuals_provider
{
    std::vector<motion_measurement>::const_iterator _begin, _end;
    // rotator _rotator;
    // std::vector<geometry> const &_geometries;

public:
    model_measurer(std::vector<motion_measurement>::const_iterator mbegin,
                   std::vector<motion_measurement>::const_iterator mend)
        : _begin{mbegin},
          _end{mend}
    {
    }
    // model_measurer(std::vector<motion_measurement>::const_iterator mbegin,
    //                std::vector<motion_measurement>::const_iterator mend,
    //                std::vector<rotation_measurement>::const_iterator rbegin,
    //                std::vector<rotation_measurement>::const_iterator rend,
    //                std::vector<geometry> const &geometries)
    //     : model_measurer(mbegin, mend),
    //       _rotator{rbegin, rend},
    //       _geometries{geometries}
    // {
    // }
    math::vector get_residuals(math::vector const &v) const override
    {
        auto f = _make_forecast(v);
        math::vector rv(std::distance(_begin, _end) * 6);
        std::size_t index{};
        for (auto iter = _begin; iter != _end; ++iter)
        {
            auto p = f.point(time_to_number(iter->t));
            for (std::size_t i{}; i < 6; ++i)
            {
                rv[index++] = iter->v[i] - p[i];
            }
        }
        return rv;
    }

    void get_residuals_and_derivatives(math::vector const &v, math::vector &rv, math::matrix &dm) const
    {
        auto f = _make_forecastext(v);
        rv = math::vector(6 * std::distance(_begin, _end));
        dm = math::matrix(v.size(), rv.size());
        std::size_t index{};
        for (auto iter = _begin; iter != _end; ++iter)
        {
            auto p = f.point(time_to_number(iter->t));
            for (std::size_t i{}; i < 6; ++i)
            {
                rv[index] = iter->v[i] - p[i];
                for (std::size_t j{}; j < dm.rows(); ++j)
                {
                    dm[j][index] = p[6 + j * v.size() + i];
                }
                ++index;
            }
        }
    }

private:
    forecast _make_forecast(math::vector const &in) const
    {
        math::vec6 v;
        std::memcpy(v.data(), in.data(), sizeof(v));
        return make_forecast(v,
                             _begin->t,
                             (_end - 1)->t,
                             in[6]);
    }
    forecastext _make_forecastext(math::vector const &in) const
    {
        vec55 v;
        std::memcpy(v.data(), in.data(), sizeof(double) * 6);
        for (auto ptr = v.data() + 6; ptr < v.data() + v.size(); ptr += in.size() + 1)
        {
            *ptr = 1;
        }
        return make_forecast(v,
                             _begin->t,
                             (_end - 1)->t,
                             in[6]);
    }
};

class model_variator : public math::variator
{
public:
    math::vector get_variations() const override
    {
        return math::vector{
            1,
            1,
            1,
            1e-2,
            1e-2,
            1e-2,
            1e-6,
        };
    }
};

auto to_vec6(double const *v)
{
    return math::vec6{v[0], v[1], v[2], v[3], v[4], v[5]};
}

double reflection_coefficient(double square, double refl)
{
    return square * (1 + refl);
}

class residual_iterator
{
protected:
    math::vector const &_v;
    std::size_t _pos;

public:
    residual_iterator(math::vector const &v, std::size_t pos) : _v{v}, _pos{pos}
    {
    }
    bool operator!=(residual_iterator const &other) const noexcept
    {
        return _pos != other._pos;
    }
    residual_iterator &operator++() noexcept
    {
        _pos += 6;
        return *this;
    }
    friend std::size_t distance(residual_iterator const &begin, residual_iterator const &end)
    {
        return (end._pos - begin._pos) / 6;
    }
};

class radiusvec_iterator : public residual_iterator
{
public:
    using residual_iterator::residual_iterator;
    double operator*() const
    {
        double rad{};
        for (std::size_t i{}; i < 3; ++i)
        {
            rad += math::sqr(_v[_pos + i]);
        }
        return std::sqrt(rad);
    }
    radiusvec_iterator &operator++() noexcept
    {
        residual_iterator::operator++();
        return *this;
    }
};

class velocity_iterator : public residual_iterator
{
public:
    using residual_iterator::residual_iterator;
    double operator*() const
    {
        double rad{};
        for (std::size_t i{3}; i < 6; ++i)
        {
            rad += math::sqr(_v[_pos + i]);
        }
        return std::sqrt(rad);
    }
    velocity_iterator &operator++() noexcept
    {
        residual_iterator::operator++();
        return *this;
    }
};

auto begin_radiusvec(math::vector const &v)
{
    return radiusvec_iterator{v, 0};
}

auto end_radiusvec(math::vector const &v)
{
    return radiusvec_iterator{v, v.size()};
}

auto begin_velocity(math::vector const &v)
{
    return velocity_iterator{v, 0};
}

auto end_velocity(math::vector const &v)
{
    return velocity_iterator{v, v.size()};
}

void print_mean_std(std::ostream &os, double mean, double std)
{
    // os << "среднее значение = " << mean << " СКО = " << std << std::endl;
    os << "медиана = " << mean << " СКО = " << std << std::endl;
}

std::ostream &operator<<(std::ostream &os, math::iteration const &iter)
{
    os << "===== Итерация № " << iter.n << " =====" << std::endl;
    os << "Вектор параметров ";
    print_vec<vecsize>(os, iter.v.data());
    os << std::endl
       << "Вектор поправок ";
    if (iter.dv.size() == vecsize)
        print_dvec<vecsize>(os, iter.dv.data());
    os << std::endl
       << "Ф-ция невязок" << equal << iter.r << std::endl;
    // os << "Вектор невязок" << std::endl;
    // for (std::size_t i{}, n{1}; i < iter.rv.size(); ++n)
    // {
    //     os << "№" << n << ' ';
    //     for (std::size_t k{}; k < 6; ++k)
    //     {
    //         os << std::setw(column_width) << iter.rv[i++] << ' ';
    //     }
    //     os << std::endl;
    // }
    return os;
}

void print_iterations(std::ostream &os, computation_logger const &logger)
{
    os << "Кол-во итераций оптимизации " << logger.size() << std::endl;
    for (auto &iter : logger)
    {
        os << iter;
    }
}

void print_radiusvec_residual(std::ostream &os, double mean, double std)
{
    os << "Невязка по радиусу-вектору: ";
    print_mean_std(os, mean, std);
}

void print_velocity_residual(std::ostream &os, double mean, double std)
{
    os << "Невязка по скорости: ";
    print_mean_std(os, mean, std);
}

template <typename iterator>
double get_median(iterator begin, iterator end)
{
    std::vector<double> arr;
    while (begin != end)
    {
        arr.push_back(*begin);
        ++begin;
    }
    std::sort(std::begin(arr), std::end(arr));
    return arr[arr.size() / 2];
}

template <typename iterator>
void median_std(double &mean, double &std, iterator begin, iterator end)
{
    math::mean_std(mean, std, begin, end);
    mean = get_median(begin, end);
}

auto print_statistic_(std::ostream &os, math::iteration const &iter)
{
    auto &v = iter.rv;
    double rad_mean{}, rad_std{};
    double vel_mean{}, vel_std{};
    // math::mean_std(rad_mean, rad_std, begin_radiusvec(v), end_radiusvec(v));
    // math::mean_std(vel_mean, vel_std, begin_velocity(v), end_velocity(v));
    median_std(rad_mean, rad_std, begin_radiusvec(v), end_radiusvec(v));
    median_std(vel_mean, vel_std, begin_velocity(v), end_velocity(v));
    print_radiusvec_residual(os, rad_mean, rad_std);
    print_velocity_residual(os, vel_mean, vel_std);
}

template <typename input_iterator, typename output_iterator>
void fwdcopy(input_iterator input_begin, input_iterator input_end, output_iterator output_begin)
{
    while (input_begin != input_end)
    {
        *output_begin = *input_begin;
        ++input_begin;
        ++output_begin;
    }
}

void show_figure(computation_logger const &logger)
{
    std::vector<double> x, y1, y2;
    auto &first_iter = logger.front();
    auto &last_iter = logger.back();
    fwdcopy(begin_radiusvec(first_iter.rv), end_radiusvec(first_iter.rv), std::inserter(y1, std::end(y1)));
    fwdcopy(begin_radiusvec(last_iter.rv), end_radiusvec(last_iter.rv), std::inserter(y2, std::end(y2)));
    x.resize(y1.size());
    for (std::size_t i{}; i < x.size(); ++i)
    {
        x[i] = i + 1.0;
    }
    figure_provider::show_residuals(x.data(), y1.data(), x.data(), y2.data(), x.size());
}

template <typename iterator>
std::size_t filtrate(std::vector<motion_measurement> &measurements, iterator begin, iterator end)
{
    double median = get_median(begin, end);
    std::size_t index{}, count{};
    for (; begin != end; ++begin, ++index)
    {
        if (*begin >= 3 * median)
        {
            measurements.erase(measurements.begin() + index);
            ++count;
        }
    }
    return count;
}

void filtrate(std::vector<motion_measurement> &measurements, math::iteration const &iter, std::ostream &os)
{
    os << "Отбракованные измерения:\n";
    os << "По радиусу-вектору " << filtrate(measurements, begin_radiusvec(iter.rv), end_radiusvec(iter.rv)) << std::endl;
    os << "По модулю скорости " << filtrate(measurements, begin_velocity(iter.rv), end_velocity(iter.rv)) << std::endl;
}

void compute_motion(math::vector v,
                    std::vector<motion_measurement> &measurements,
                    std::ostream &os,
                    bool filtration = false)
{
    constexpr std::size_t iterations{20};
    std::exception_ptr exptr;
    model_measurer meas{std::begin(measurements), std::end(measurements)};
    computation_logger logger;
    logger.reserve(iterations);
    try
    {
        math::levmarq(v, meas, &logger, 1e-3, iterations);
    }
    catch (const std::exception &ex)
    {
        os << ex.what() << std::endl;
        using namespace std::string_literals;
        exptr = std::make_exception_ptr(std::runtime_error("Error occured while computation. "s + ex.what()));
    }
    if (!logger.empty())
    {
        os << "Невязки до оптимизации" << std::endl;
        print_statistic_(os, logger.front());
        os << "Оптимизированные параметры движения ";
        print_vec<vecsize>(os, v.data());
        os << std::endl;
        os << "Невязки после оптимизации" << std::endl;
        print_statistic_(os, logger.back());
        os << std::endl;
        print_iterations(os, logger);
        if (!filtration)
            show_figure(logger);
    }
    if (exptr)
    {
        std::rethrow_exception(exptr);
    }
    if (filtration)
    {
        filtrate(measurements, logger.back(), os);
    }
}

auto end_iterator(std::vector<motion_measurement>::const_iterator begin,
                  std::vector<motion_measurement>::const_iterator end)
{
    return std::lower_bound(begin, end, begin->t + std::chrono::days(1),
                            [](motion_measurement const &m, time_type t)
                            { return m.t < t; });
}

void application_configurer::compute() const
{
    std::vector<motion_measurement> measurements(_mbegin, end_iterator(_mbegin, _mend));
    math::vector v(vecsize);
    std::copy(_mbegin->v, _mbegin->v + 6, v.begin());
    model_measurer meas{std::begin(measurements), std::end(measurements)}; //, _rbegin, _rend, _geometries};
    computation_logger logger;
    auto os = open_outfile(_computation_filepath);
    os << std::fixed;
    os << "T = ";
    write_to_stream(os, _mbegin->t);
    os << std::endl
       << "Исходные параметры движения ";
    print_vec<vecsize>(os, v.data());
    os << std::endl;
    compute_motion(v, measurements, os, true);
    os << std::endl;
    compute_motion(v, measurements, os);
}