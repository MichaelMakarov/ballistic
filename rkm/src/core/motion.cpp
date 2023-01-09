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
        double da = 2 * pi - p.da;
        if (std::abs(da) < std::abs(p.da))
            p.da = da;
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
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const motion_params &mp)
{
    os << mp.t << ' ';
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

class basic_model_wrapper : public optimization_interface<6, 2>
{
    measuring_interval const &_inter;
    time_h _tn;

public:
    basic_model_wrapper(measuring_interval const &inter, time_h tn) : _inter{inter}, _tn{tn} {}

    std::size_t points_count() const override
    {
        return _inter.points_count();
    }

    vec<6> variations() const override
    {
        vec<6> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        return v;
    }

    void update(double &value, size_t index, double add) const override
    {
        value += add;
    }

    void residual(vec<6> const &v, array_view<2> *const r) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto res = compute_basic(mp, _inter);
        for (std::size_t i{}; i < res.size(); ++i)
        {
            auto &p = res[i];
            r[i][0] = p.di;
            r[i][1] = p.da;
        }
    }
};

class extbasic_model_wrapper : public optimization_interface<7, 2>
{
    measuring_interval const &_inter;
    time_h _tn;
    double _mass;

public:
    extbasic_model_wrapper(measuring_interval const &inter, time_h tn, double m) : _inter{inter}, _tn{tn}, _mass{m} {}

    std::size_t points_count() const override
    {
        return _inter.points_count();
    }

    vec<7> variations() const override
    {
        vec<7> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        v[6] = 1;
        return v;
    }

    void update(double &value, size_t index, double add) const override
    {
        value += add;
        if (index == 6)
            value = std::min(1e2, std::max(0.0, value));
        if (index > 6)
            throw std::out_of_range("Индекс параметра за пределами диапазона.");
    }

    void residual(vec<7> const &v, array_view<2> *const r) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto res = compute_extbasic(mp, _inter, v[6], _mass);
        for (std::size_t i{}; i < res.size(); ++i)
        {
            auto &p = res[i];
            r[i][0] = p.di;
            r[i][1] = p.da;
        }
    }
};

class extended_model_wrapper : public optimization_interface<7, 2>
{
    measuring_interval const &_inter;
    time_h _tn;
    rotator const &_rot;
    round_plane const &_plane;

public:
    extended_model_wrapper(measuring_interval const &inter, time_h tn, rotator const &r, round_plane const &p) : _inter{inter}, _tn{tn}, _rot{r}, _plane{p} {}

    std::size_t points_count() const override
    {
        return _inter.points_count();
    }

    vec<7> variations() const override
    {
        vec<7> v;
        v[0] = v[1] = v[2] = position_var * 4;
        v[3] = v[4] = v[5] = velocity_var;
        v[6] = 1;
        return v;
    }

    void update(double &value, size_t index, double add) const override
    {
        value += add;
        if (index == 6)
            value = std::min(1e2, std::max(0.0, value));
        if (index > 6)
            throw std::out_of_range("Индекс параметра за пределами диапазона.");
    }

    void residual(vec<7> const &v, array_view<2> *const r) const override
    {
        motion_params mp{.v = subv<0, 5>(v), .t = _tn};
        auto p = _plane;
        p.square = v[6];
        auto res = compute_extended(mp, _inter, _rot, make_plane(p));
        for (std::size_t i{}; i < res.size(); ++i)
        {
            auto &p = res[i];
            r[i][0] = p.di;
            r[i][1] = p.da;
        }
    }
};

constexpr std::size_t iterations{30};

void estimate_model(measuring_interval const &inter, time_h t, basic_info &info)
{
    basic_model_wrapper mwb(inter, t);
    levenberg_marquardt(info.v, mwb, &info.l, 1e-3, iterations);
}

void estimate_model(measuring_interval const &inter, time_h t, round_plane const &p, extbasic_info &info)
{
    vec<7> v;
    auto end = std::copy(info.v.data(), info.v.data() + info.v.size(), v.data());
    v[6] = (1 + p.refl) * p.square;
    extbasic_model_wrapper mw(inter, t, p.mass);
    levenberg_marquardt(v, mw, &info.l, 1e-3, iterations);
    std::copy(v.data(), end, info.v.data());
    info.s = v[6];
}

void estimate_model(measuring_interval const &inter, time_h t, round_plane const &p, rotator const &r, extended_info &info)
{
    vec<7> v;
    auto end = std::copy(info.v.data(), info.v.data() + info.v.size(), v.data());
    v[6] = p.square;
    extended_model_wrapper mw(inter, t, r, p);
    levenberg_marquardt(v, mw, &info.l, 1e-3, iterations);
    std::copy(v.data(), end, info.v.data());
    info.s = v[6];
}