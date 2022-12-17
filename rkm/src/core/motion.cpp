#include <motion.hpp>
#include <forecast.hpp>
#include <transform.hpp>
#include <models.hpp>
#include <future>
#include <array>

/**
 * @brief Вариация для координаты
 *
 */
constexpr double position_var{25};
/**
 * @brief Вариация для скорости
 *
 */
constexpr double velocity_var{0.25};
/**
 * @brief Вариация для угла
 *
 */
constexpr auto angle_var{deg_to_rad(3.6)};

struct motion_point
{
    double i;
    double a;
    double di;
    double da;
};

auto compute_motion_residuals(const forecast &f, const measuring_interval &inter)
{
    std::vector<motion_point> arr(inter.points_count());
    auto begin = std::begin(inter);
    auto end = std::end(inter);
    for (size_t i{}; begin != end; ++begin)
    {
        auto &meas = begin.measurement();
        const auto mp = f.point(meas.t);
        double sph[3]{};
        transform<abs_cs, sph_cs, grw_cs, ort_cs>::backward(mp.v.data(), sidereal_time_mean(mp.t), sph);
        auto &p = arr[i++];
        p.i = sph[1];
        p.a = sph[2];
        p.di = meas.i - p.i;
        p.da = meas.a - p.a;
    }
    return arr;
}

auto compute_basic(const motion_params &mp, const measuring_interval &inter)
{
    return compute_motion_residuals(make_forecast(mp, inter.tk()), inter);
}

auto compute_extbasic(motion_params const &mp, measuring_interval const &inter, double s, double m)
{
    return compute_motion_residuals(make_forecast(mp, inter.tk(), s, m), inter);
}

auto compute_extended(const motion_params &mp, const measuring_interval &inter, const rotator &r, const object_model &o)
{
    return compute_motion_residuals(make_forecast(mp, inter.tk(), r, o), inter);
}

object_model make_plane(const round_plane &p)
{
    object_model obj;
    obj.mass = p.mass;
    obj.surface.resize(2);
    for (auto &face : obj.surface)
    {
        face.refl = p.refl;
        face.square = p.square;
    }
    obj.surface[0].norm = p.normal;
    obj.surface[1].norm = -obj.surface[0].norm;
    return obj;
}

//-------------------------------------------------------

auto make_round_plane(double mass, double rsquare, double refl, const vec3 &normal)
{
    return round_plane{
        .mass = mass,
        .square = rsquare / refl,
        .refl = refl,
        .normal = normal,
    };
}

#include <fstream>
#include <timefmt.hpp>
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const motion_params &mp)
{
    os << std::format("{}", mp.t) << ' ';
    for (size_t i{}; i < mp.v.size(); ++i)
        os << mp.v[i] << ' ';
    double buf[3]{};
    transform<abs_cs, sph_cs, grw_cs, ort_cs>::backward(mp.v.data(), sidereal_time_mean(mp.t), buf);
    os << "i = " << rad_to_deg(buf[1]) << " a = " << rad_to_deg(buf[2]);
    return os;
}

void compare_models(vec6 const &v, time_h tn, time_h tk, rotator const &rot, round_plane const &plane)
{
    auto obj = make_plane(plane);
    motion_params mp{.v = v, .t = tn};
    auto f1 = make_forecast(mp, tk);
    auto f2 = make_forecast(mp, tk, (1 + plane.refl) * plane.square, plane.mass);
    // auto f2 = make_forecast(mp, tk, rot, obj);
    std::ofstream fout("log_forecasts.txt");
    fout << std::setprecision(16) << std::fixed;
    for (size_t i{}; i < f1._points.size(); ++i)
    {
        fout << "point " << i + 1 << '\n';
        fout << f1._points[i] << '\n';
        fout << f2._points[i] << '\n';
    }
}

//-----------------------------------------------------

class basic_model_wrapper : public optimization_interface<6>
{
    measuring_interval const &_inter;
    time_h _tn;

public:
    basic_model_wrapper(measuring_interval const &inter, time_h tn) : _inter{inter}, _tn{tn} {}

    vec<6> variations() const override
    {
        vec<6> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        return v;
    }

    double derivative(double deriv, size_t index) const override
    {
        return deriv;
    }

    double residual(vec<6> const &v) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto res = compute_basic(mp, _inter);
        double val{};
        for (auto &p : res)
            val += sqr(p.di) + sqr(p.da);
        return val;
    }
};

class extbasic_model_wrapper : public optimization_interface<7>
{
    measuring_interval const &_inter;
    time_h _tn;
    double _mass;

public:
    extbasic_model_wrapper(measuring_interval const &inter, time_h tn, double m) : _inter{inter}, _tn{tn}, _mass{m} {}

    vec<7> variations() const override
    {
        vec<7> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        v[6] = 1;
        return v;
    }

    double derivative(double deriv, size_t index) const override
    {
        if (index == 6)
            deriv = sign(deriv);
        if (index > 6)
            throw std::out_of_range("Индекс параметра за пределами диапазона.");
        return deriv;
    }

    double residual(vec<7> const &v) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto res = compute_extbasic(mp, _inter, v[6], _mass);
        double val{};
        for (auto &p : res)
            val += sqr(p.di) + sqr(p.da);
        return val;
    }
};

#include <iostream>

void estimate_basic_model(measuring_interval const &inter, time_h t, round_plane const &p, basic_info &info)
{
    basic_logger<6> l;
    vec<6> v{info.v};
    basic_model_wrapper mwb(inter, t);
    levenberg_marquardt(v, mwb, &l);
    std::cout << "Model 1 residual = " << l.back().r << std::endl;
    vec<7> params;
    auto end = std::copy(info.v.data(), info.v.data() + info.v.size(), params.data());
    params[6] = (1 + p.refl) * p.square;
    extbasic_model_wrapper mw(inter, t, p.mass);
    levenberg_marquardt(params, mw, &info.l);
    std::copy(params.data(), end, info.v.data());
    info.s = params[6];
    std::cout << "Model 2 residual = " << info.l.back().r << std::endl;
}

class extended_model_wrapper : public optimization_interface<7>
{
    measuring_interval const &_inter;
    time_h _tn;
    rotator const &_rot;
    round_plane const &_plane;

public:
    extended_model_wrapper(measuring_interval const &inter, time_h tn, rotator const &r, round_plane const &p) : _inter{inter}, _tn{tn}, _rot{r}, _plane{p} {}

    vec<7> variations() const override
    {
        vec<7> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        v[6] = 1;
        return v;
    }

    double derivative(double deriv, size_t index) const override
    {
        if (index == 6)
            deriv = sign(deriv);
        if (index > 7)
            throw std::out_of_range("Индекс параметра за пределами диапазона.");
        return deriv;
    }

    double residual(vec<7> const &v) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto p = _plane;
        p.square = v[6];
        auto res = compute_extended(mp, _inter, _rot, make_plane(p));
        double val{};
        for (auto &p : res)
            val += sqr(p.di) + sqr(p.da);
        return val;
    }
};

void estimate_extended_model(measuring_interval const &inter, time_h t, round_plane const &p, rotator const &r, extended_info &info)
{
    vec<7> params;
    auto end = std::copy(info.v.data(), info.v.data() + info.v.size(), params.data());
    params[6] = p.square;
    extended_model_wrapper mw(inter, t, r, p);
    levenberg_marquardt(params, mw, &info.l);
    std::copy(params.data(), end, info.v.data());
    info.square = params[6];
    std::cout << "Model 3 residual = " << info.l.back().r << std::endl;
}