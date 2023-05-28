#include <config.hpp>
#include <fileutility.hpp>
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

class model_measurer : public math::measurer
{
    std::vector<motion_measurement>::const_iterator _begin, _end;
    rotator _rotator;
    std::vector<geometry> const &_geometries;

public:
    model_measurer(std::vector<motion_measurement>::const_iterator mbegin,
                   std::vector<motion_measurement>::const_iterator mend,
                   std::vector<rotation_measurement>::const_iterator rbegin,
                   std::vector<rotation_measurement>::const_iterator rend,
                   std::vector<geometry> const &geometries)
        : _begin{mbegin}, _end{mend},
          _rotator{rbegin, rend},
          _geometries{geometries}
    {
        _end = std::lower_bound(_begin, _end, _begin->t + std::chrono::days(1),
                                [](motion_measurement const &m, time_point_t t)
                                { return m.t < t; });
    }
    math::vector get_residuals(math::vector const &v) const override
    {
        math::vec6 mv;
        std::copy(v.begin(), v.begin() + 6, mv.data());
        auto f = make_forecast(mv,
                               clock_type::to_time_t(_begin->t),
                               clock_type::to_time_t((_end - 1)->t),
                               v[6],
                               _geometries,
                               _rotator);
        math::vector rv(std::distance(_begin, _end) * 6);
        std::size_t index{};
        for (auto iter = _begin; iter != _end; ++iter)
        {
            auto &m = *iter;
            auto p = f.point(clock_type::to_time_t(m.t));
            for (std::size_t i{}; i < 6; ++i)
            {
                rv[index++] = m.v[i] - p[i];
            }
        }
        return rv;
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

void print_statistic_(std::ostream &os, math::iteration const &iter)
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

void application_configurer::compute() const
{
    auto os = open_outfile(_computation_filepath);
    os << std::fixed;
    math::vector v(vecsize);
    std::copy(_mbegin->v, _mbegin->v + 6, v.begin());
    std::size_t iterations{20};
    model_measurer meas{_mbegin, _mend, _rbegin, _rend, _geometries};
    model_variator var;
    computation_logger logger;
    logger.reserve(iterations);
    os << "T = ";
    write_to_stream(os, _mbegin->t);
    os << std::endl
       << "Исходные параметры движения ";
    print_vec<vecsize>(os, v.data());
    os << std::endl;
    std::exception_ptr exptr;
    try
    {
        math::levmarq(v, meas, var, nullptr, &logger, 1e-2, iterations);
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
        show_figure(logger);
    }
    if (exptr)
    {
        std::rethrow_exception(exptr);
    }
}