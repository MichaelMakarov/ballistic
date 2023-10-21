#include <motion.hpp>

#include <forecast.hpp>
#include <transform.hpp>
#include <ball.hpp>
#include <optimization.hpp>

constexpr std::size_t _res_size{2};

constexpr double vararr[]{
    25,
    25,
    25,
    .25,
    .25,
    .25,
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

void run_optimization(measuring_interval const &inter, orbit_data &data, math::iterations_saver &saver, std::size_t iter_count)
{
    math::vector v = make_vector(data, 6);
    model_measurer meas{inter, data.t};
    parameters_variator var;
    math::levmarq(v, meas, var, nullptr, &saver, 1e-5, iter_count);
}

void run_optimization_s(measuring_interval const &inter, orbit_data &d, math::iterations_saver &saver, std::size_t iter_count)
{
    math::vector v = make_vector(d, 7);
    motion_residuals res{inter, d.t};
    math::levmarq(v, res, &saver, 1e-5, iter_count);
}
