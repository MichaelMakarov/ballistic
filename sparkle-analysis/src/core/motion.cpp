#include <motion.hpp>
#include <forecast.hpp>
#include <transform.hpp>
#include <ball.hpp>
#include <optimization.hpp>
#include <ostream>

constexpr std::size_t _res_size{2};

constexpr double vararr[]{
    25,
    25,
    25,
    .25,
    .25,
    .25,
};

template <std::size_t _index, bool = _index<_res_size> class residual_iterator;

template <std::size_t _index>
class residual_iterator<_index, true>
{
    math::vector const *_v;
    std::size_t _offset;

public:
    residual_iterator(math::vector const &v, std::size_t offset) : _v{&v}, _offset{offset}
    {
    }
    residual_iterator(residual_iterator const &) = default;
    residual_iterator &operator=(residual_iterator const &) = default;
    residual_iterator &operator++()
    {
        _offset += _res_size;
        return *this;
    }
    double operator*() const
    {
        return (*_v)[_offset + _index];
    }
    bool operator!=(residual_iterator const &other) const
    {
        return _v != other._v || _offset != other._offset;
    }
};

std::ostream &operator<<(std::ostream &os, math::vector const &v)
{
    for (std::size_t i{}; i < v.size(); ++i)
    {
        os << v[i] << ' ';
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, math::iteration const &iter)
{
    os << "Итерация №" << iter.n << '\n';
    os << "Ф-ция невязок " << math::rad_to_deg(iter.r) << '\n';
    os << "Вектор параметров " << iter.v << '\n';
    os << "Вектор поправок " << iter.dv << '\n';
    return os;
}

void print_stat(std::ostream &os, char const *name, double mean, double std)
{
    os << name << " \t среднее значение " << math::rad_to_deg(mean) << "град \t СКО " << math::rad_to_deg(std) << "град\n";
}

void print_stat_info(std::ostream &os, math::iteration const &iter)
{
    double mean, std;
    math::mean_std(mean, std, residual_iterator<0>(iter.rv, 0), residual_iterator<0>(iter.rv, iter.rv.size()));
    print_stat(os, "Склонение:\t", mean, std);
    math::mean_std(mean, std, residual_iterator<1>(iter.rv, 0), residual_iterator<1>(iter.rv, iter.rv.size()));
    print_stat(os, "Восхождение:\t", mean, std);
}

class optimization_logger : public math::iterations_saver
{
    std::ostream &_os;
    math::iteration _first, _last;
    std::size_t _iter_index{};

public:
    optimization_logger(std::ostream &os, measuring_interval const &inter) : _os{os}
    {
        _os << std::fixed << std::chrono::system_clock::now() << " Начало записи протокола вычислений.\n\n";
        _os << "Мерный интервал: " << inter.tn() << " - " << inter.tk() << " " << inter.points_count() << " измерений.\n";
    }
    ~optimization_logger()
    {
        _os << "Статистические параметры невязок.\n";
        _os << "На первой итерации\n";
        print_stat_info(_os, _first);
        _os << "На последней итерации (" << _last.n << ")\n";
        print_stat_info(_os, _last);
        _os << "\n\nОкончание записи протокола вычислений. " << std::chrono::system_clock::now();
    }
    void save(math::iteration &&iter)
    {
        _os << iter << '\n';
        if (_iter_index == 0)
        {
            _first = std::move(iter);
        }
        else
        {
            _last = std::move(iter);
        }
        ++_iter_index;
    }
};

double absmin(double left, double right)
{
    return std::abs(left) < std::abs(right) ? left : right;
}

math::vector make_vector(orbit_data const &d, std::size_t size = 7)
{
    math::vector v(size);
    std::memcpy(v.data(), d.v, sizeof(d.v));
    return v;
}

class model_measurer : public math::measurer
{
    measuring_interval _inter;
    time_type _t;

public:
    model_measurer(measuring_interval const &inter, time_type t) : _inter{inter}, _t{t} {}
    math::vector get_residuals(math::vector const &v) const override
    {
        auto f = _make_forecast(v);
        math::vector rv(_inter.points_count() * 2);
        auto begin = _inter.begin();
        auto end = _inter.end();
        for (std::size_t i{}; begin != end; ++begin)
        {
            auto &meas = begin.measurement();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(meas.t.time_since_epoch()).count();
            auto p = f.point(ms);
            double sph[3];
            transform<abs_cs, sph_cs, grw_cs, ort_cs>::backward(p.data(), sidereal_time(ms / 1000), sph);
            rv[i++] = meas.i - sph[1];
            double da = meas.a - sph[2];
            rv[i++] = absmin(da, 2 * math::pi - da);
        }
        return rv;
    }

private:
    forecast _make_forecast(math::vector const &in) const
    {
        math::vec6 v;
        std::memcpy(v.data(), in.data(), sizeof(v));
        return make_forecast(v, _t, _inter.tk(), 0); // in[6]);
    }
};

class parameters_variator : public math::variator
{
public:
    math::vector get_variations() const override
    {
        return math::vector{
            25,
            25,
            25,
            .25,
            .25,
            .25,
            // 1,
        };
    }
};

void run_optimization(measuring_interval const &inter, orbit_data &data, std::ostream &os)
{
    math::vector v = make_vector(data, 6);
    model_measurer meas{inter, data.t};
    parameters_variator var;
    optimization_logger log{os, inter};
    math::levmarq(v, meas, var, nullptr, &log, 1e-5);
}

void diffsphbyxyz(double const xyz[3], double df[3], double dl[3])
{
    double xy = math::sqr(xyz[0]) + math::sqr(xyz[1]);
    dl[0] = -xyz[1] / xy;
    dl[1] = xyz[0] / xy;
    dl[2] = 0;
    double rsqr = xy + math::sqr(xyz[2]);
    xy = std::sqrt(xy);
    df[0] = df[1] = -xyz[2] / (rsqr * xy);
    df[0] *= xyz[0];
    df[1] *= xyz[1];
    df[2] = xy / rsqr;
}

class motion_residuals : public math::residuals_provider
{
    using transform_t = transform<abs_cs, sph_cs, grw_cs, ort_cs>;
    measuring_interval _inter;
    time_type _t;

public:
    motion_residuals(measuring_interval const &inter, time_type t) : _inter{inter}, _t{t} {}
    void get_residuals_and_derivatives(math::vector const &v, math::vector &rv, math::matrix &mx) const override
    {
        auto f = _make_forecast_var(v);
        rv = math::vector(_inter.points_count() * 2);
        mx = math::matrix(7, rv.size());
        auto begin = _inter.begin();
        auto end = _inter.end();
        for (std::size_t i{}; begin != end; ++begin, i += 2)
        {
            auto &meas = begin.measurement();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(meas.t.time_since_epoch()).count();
            auto p = f.point(ms);
            double st = sidereal_time(ms / 1000);
            double sph[3];
            transform_t::backward(p.data(), st, sph);
            rv[i + 0] = meas.i - sph[1];
            double da = meas.a - sph[2];
            rv[i + 1] = absmin(da, 2 * math::pi - da);
            // производные широты и долготы по декартовым координатам
            double df[3], dl[3];
            diffsphbyxyz(p.data(), df, dl);
            for (std::size_t r{}; r < mx.rows(); ++r)
            {
                size_t index{6 + r * mx.rows()};
                for (size_t j{}; j < 3; ++j)
                {
                    mx[r][i + 0] += df[j] * p[index + j];
                    mx[r][i + 1] += dl[j] * p[index + j];
                }
            }
        }
    }

    math::vector get_residuals(math::vector const &v) const override
    {
        auto f = _make_forecast(v);
        math::vector rv(_inter.points_count() * 2);
        auto begin = _inter.begin();
        auto end = _inter.end();
        for (std::size_t i{}; begin != end; ++begin, i += 2)
        {
            auto &meas = begin.measurement();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(meas.t.time_since_epoch()).count();
            auto p = f.point(ms);
            double st = sidereal_time(ms / 1000);
            double sph[3];
            transform_t::backward(p.data(), st, sph);
            rv[i + 0] = meas.i - sph[1];
            double da = meas.a - sph[2];
            rv[i + 1] = absmin(da, 2 * math::pi - da);
        }
        return rv;
    }

private:
    forecast_var _make_forecast_var(math::vector const &in) const
    {
        math::vec<55> v;
        std::memcpy(v.data(), in.data(), sizeof(double) * 6);
        for (std::size_t i{}; i < 7; ++i)
        {
            v[6 + 8 * i] = 1;
        }
        return make_forecast(v, _t, _inter.tk(), in[6]);
    }

    forecast _make_forecast(math::vector const &in) const
    {
        math::vec6 v;
        std::memcpy(v.data(), in.data(), sizeof(v));
        return make_forecast(v, _t, _inter.tk(), in[6]);
    }
};

void run_optimization_s(measuring_interval const &inter, orbit_data &d, std::ostream &os)
{
    math::vector v = make_vector(d, 7);
    motion_residuals res{inter, d.t};
    optimization_logger log{os, inter};
    math::levmarq(v, res, &log, 1e-5);
}