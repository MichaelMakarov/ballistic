#include <fileutility.hpp>
#include <printutility.hpp>
#include <measurement.hpp>
#include <figure.hpp>
#include <forecast.hpp>
#include <optimize.hpp>
#include <rotator.hpp>
#include <maths.hpp>
#include <algorithm>
#include <iomanip>
#include <cstdarg>

std::size_t constexpr vecsize{7};

using optimization_interface_t = math::optimization_interface<vecsize, 6>;
using optimization_iteration_t = math::optimization_iteration<vecsize>;
using optimization_logger_t = math::optimization_logger<vecsize>;

class computation_logger : public optimization_logger_t, public std::vector<optimization_iteration_t>
{
public:
    using std::vector<optimization_iteration_t>::vector;
    void add(optimization_iteration_t &&iter) override
    {
        push_back(std::move(iter));
    }
};
template <>
struct math::residual_function<vecsize>
{
    static double residual(math::vector const &v)
    {
        return (v * v) / v.size();
    }
};

class model_optimizer : public optimization_interface_t
{
    std::vector<motion_measurement>::const_iterator _begin, _end;
    rotator _rotator;
    std::vector<geometry> const &_geometries;

public:
    model_optimizer(std::vector<motion_measurement> const &motion_meas,
                    std::vector<rotation_measurement> const &rotation_meas,
                    std::vector<geometry> const &geometries)
        : _rotator{rotation_meas},
          _geometries{geometries}
    {
        _begin = std::begin(motion_meas);
        _end = std::lower_bound(std::begin(motion_meas), std::end(motion_meas), _begin->t + std::chrono::days(1),
                                [](motion_measurement const &m, time_point_t t)
                                { return m.t < t; });
    }
    std::size_t points_count() const override { return std::distance(_begin, _end); }
    math::vec<vecsize> variations() const override
    {
        math::vec<vecsize> v;
        v[0] = v[1] = v[2] = 1;
        v[3] = v[4] = v[5] = 1e-2;
        v[6] = 1e-6;
        // v[7] = 1;
        return v;
    }
    void update(double &value, std::size_t index, double add) const override
    {
        if (index < 8)
            value += add;
    }
    void residual(math::vec<vecsize> const &v, math::array_view<6> *res) const override
    {
        auto f = make_forecast(v.subv<0, 6>(), to_time_t(_begin->t), to_time_t((_end - 1)->t), v[6], 0, ); //[7]);
        for (auto iter = _begin; iter != _end; ++iter)
        {
            auto &m = *iter;
            auto p = f.point(to_time_t(m.t));
            auto &r = *res;
            for (std::size_t i{}; i < 6; ++i)
            {
                r[i] = m.v[i] - p[i];
            }
            ++res;
        }
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
    os << "среднее значение = " << mean << " СКО = " << std << std::endl;
}

std::ostream &operator<<(std::ostream &os, optimization_iteration_t const &iter)
{
    os << "===== Итерация № " << iter.n << " =====" << std::endl;
    os << "Вектор параметров ";
    print_vec<vecsize>(os, iter.v.data());
    os << std::endl
       << "Вектор поправок ";
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

void print_statistic_(std::ostream &os, optimization_iteration_t const &iter)
{
    auto &v = iter.rv;
    double rad_mean{}, rad_std{};
    double vel_mean{}, vel_std{};
    math::mean_std(rad_mean, rad_std, begin_radiusvec(v), end_radiusvec(v));
    math::mean_std(vel_mean, vel_std, begin_velocity(v), end_velocity(v));
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

void compute_motion(std::vector<motion_measurement> const &motion_meas, fs::path const &filepath)
{
    auto os = open_outfile(filepath);
    os << std::fixed;
    auto &first = motion_meas.front();
    math::vec<vecsize> v;
    std::copy(first.v, first.v + 6, v.data());
    // v[7] = reflection_coefficient(15, 0.4);
    //  auto f = make_forecast(v.subv<0, 6>(), first.t, motion_meas.back().t, v[6]);
    std::size_t iterations{20};
    model_optimizer optimizer{motion_meas};
    computation_logger logger;
    logger.reserve(iterations);
    os << "T = ";
    write_to_stream(os, first.t);
    os << std::endl
       << "Исходные параметры движения ";
    print_vec<vecsize>(os, v.data());
    os << std::endl;
    std::exception_ptr exptr;
    try
    {
        math::levmarq(v, optimizer, &logger, 1e-5, iterations);
    }
    catch (const std::exception &ex)
    {
        os << ex.what() << std::endl;
        exptr = std::make_exception_ptr(std::runtime_error(format(("Error occured while computation. %", ex.what()))));
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
        show_figure(logger);
    }
    if (exptr)
    {
        std::rethrow_exception(exptr);
    }
}