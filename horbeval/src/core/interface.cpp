#include <interface.h>
#include <future>

forecast<6> make_forecast(const motion_params& mp, time_h tk)
{
    return make_forecast<basic_motion_model>(mp, tk);
}

forecast<6> make_forecast(const motion_params& mp, time_h tk, const rotational_params& rp, const round_plane_info& info)
{
    auto obj = make_round_plane(info);
    return make_forecast<extended_motion_model, const rotational_params&, const object_model*>(mp, tk, rp, &obj);
}

/**
 * @brief Вариация для координаты
 * 
 */
constexpr double posvar{ 25 };
/**
 * @brief Вариация для скорости
 * 
 */
constexpr double velvar{ 0.25 };
/**
 * @brief Вариация для размера поверхности
 * 
 */
constexpr double surfvar{ 0.1 };

linear_motion_interface::linear_motion_interface(const motion_params& mp, measurement_iter beg, measurement_iter end)
{
    _mp = mp;
    _beg = beg;
    _end = end;
    _tk = std::max_element(beg, end, [](const auto& left, const auto& right){ return left.t < right.t; })->t;
}

size_t linear_motion_interface::points_count() const
{
    return static_cast<size_t>(std::distance(_beg, _end));
}
    
void basic_linear_interface::update(array_view<6> correction)
{
    for (size_t i{}; i < 6; ++i) _mp.v[i] += correction[i];
}
    
void basic_linear_interface::compute(std::vector<array_view<3>>& resid, std::vector<std::array<array_view<3>, 6>>& deriv) const 
{
    constexpr double variations[]{
        posvar, posvar, posvar, velvar, velvar, velvar
    };
    auto compute_func = &make_forecast<basic_motion_model>;
    std::array<std::future<forecast<6>>, 6> forecasts;
    for (size_t i{}; i < forecasts.size(); ++i) {
        auto mp = _mp;
        mp.v[i] += variations[i];
        forecasts[i] = std::async(std::launch::async, compute_func, mp, _tk);
    }
    auto forecast = compute_func(_mp, _tk);

    std::vector<motion_params> mplist(resid.size());
    size_t point_index{};
    for (auto it = _beg; it != _end; ++it, ++point_index) {        
        mplist[point_index] = forecast.params(it->t);        
        for (size_t coord_index{}; coord_index < 3; ++coord_index) {
            resid[point_index][coord_index] = it->v[coord_index] - mplist[point_index].v[coord_index];
        }
    }

    for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
        forecast = forecasts[param_index].get();
        for (point_index = 0; point_index < mplist.size(); ++point_index) {
            auto mp = forecast.params(mplist[point_index].t);
            for (size_t coord_index{}; coord_index < 3; ++coord_index) {
                deriv[point_index][param_index][coord_index] =
                    (mp.v[coord_index] - mplist[point_index].v[coord_index]) / variations[param_index];
            }
        }
    }
}
    
extended_linear_interface::extended_linear_interface(const motion_params& mp, const rotational_params& rp, const round_plane_info& info, measurement_iter beg, measurement_iter end)
    : linear_motion_interface(mp, beg, end)
{
    _rp = rp;
    _plane = info;
}
    
void extended_linear_interface::update(array_view<7> correction)
{
    for (size_t i{}; i < 6; ++i) _mp.v[i] += correction[i];
    _plane.rad += correction[6];
}

void extended_linear_interface::compute(std::vector<array_view<3>>& resid, std::vector<std::array<array_view<3>, 7>>& deriv) const 
{
    constexpr double variations[]{
        posvar, posvar, posvar, velvar, velvar, velvar, surfvar
    };
    auto compute_func = &make_forecast<extended_motion_model, const rotational_params&, const object_model*>;
    std::array<std::future<forecast<6>>, 7> forecasts;
    auto obj = make_round_plane(_plane);
    for (size_t i{}; i < forecasts.size() - 1; ++i) {
        auto mp = _mp;
        mp.v[i] += variations[i];
        forecasts[i] = std::async(std::launch::async, compute_func, mp, _tk, _rp, &obj);
    }
    auto planev = _plane;
    planev.rad += variations[6];
    auto objv = make_round_plane(planev);
    forecasts[6] = std::async(std::launch::async, compute_func, _mp, _tk, _rp, &objv);
    auto forecast = compute_func(_mp, _tk, _rp, &obj);

    std::vector<motion_params> mplist(resid.size());
    size_t point_index{};    
    for (auto it = _beg; it != _end; ++it, ++point_index) {        
        mplist[point_index] = forecast.params(it->t);        
        for (size_t coord_index{}; coord_index < 3; ++coord_index) {
            resid[point_index][coord_index] = it->v[coord_index] - mplist[point_index].v[coord_index];
        }
    }

    for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
        forecast = forecasts[param_index].get();
        for (point_index = 0; point_index < mplist.size(); ++point_index) {
            auto mp = forecast.params(mplist[point_index].t);
            for (size_t coord_index{}; coord_index < 3; ++coord_index) {
                deriv[point_index][param_index][coord_index] =
                    (mp.v[coord_index] - mplist[point_index].v[coord_index]) / variations[param_index];
            }
        }
    }
}

#include <transform.h>

vec3 grw_to_abs(double st, const double* const in)
{
    vec3 out;
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(in, st, out.data());
    return out;
}

vec3 normal(time_h t, const double* const o, const double* const s)
{
    vec3 sun;
    solar_model::coordinates(t, sun.data());
    auto st = sidereal_time_mean(t);
    auto obs = grw_to_abs(st, o);
    auto sat = grw_to_abs(st, s);
    sun -= sat;
    obs -= sat;
    normalize(sun);
    normalize(obs);
    sat = sun + obs;
    normalize(sat);
    return sat;
}

vec3 normal(const rotation_observation& o, const vec6& v)
{
    return normal(o.t, o.o, v.data());
}

#include <arithmetics.h>
#include <conversion.h>

rotational_params estimate_rotation(
    const motion_params& mp, observation_iter beg, observation_iter end, const vec3& v,
    std::streambuf* strbuf
)
{
    using transform_t = transform<abs_cs, sph_cs, abs_cs, ort_cs>;
    wrapping_logger log{ strbuf };
    auto tn = beg->t;
    auto f = make_forecast<basic_motion_model>(mp, (end - 1)->t);
    auto norm = normal(*beg, f.params(tn).v);
    size_t count = std::distance(++beg, end);

    double time{};
    double asc{}, incl{}, vel{};
    double buf[3];

    log << "Оценка параметров вращения по измерениям в кол-ве " << count << std::endl;

    for (; beg != end; ++beg) {
        auto [axis, angle] = from_quaternion(rotation(norm, normal(*beg, f.params(beg->t).v)));
        // ось z оси вращения должна быть положительной
        if (axis[2] < 0) {
            axis = -axis;
            angle = -angle;
        }
        transform_t::backward(axis.data(), buf);
        auto t = beg->t - tn;
        time += sqr(t);
        vel += angle * t;
        incl += buf[1];
        asc += buf[2];
        log << beg->t << 
            " скл = " << rad_to_deg(buf[1]) << 
            " восх = " << rad_to_deg(buf[2]) << 
            " угол = " << rad_to_deg(angle) << std::endl;
    }

    incl /= count;
    asc /= count;
    vel /= time;
    buf[0] = 1;
    buf[1] = incl;
    buf[2] = asc;

    log << 
        "Итоговые параметры вращения" << std::endl << 
        "ось: скл = " << rad_to_deg(incl) << 
        " восх = " << rad_to_deg(asc) << 
        " угл.скор = " << rad_to_deg(vel) << " град/с" << std::endl;

    rotational_params rp;
    rp.quat = rotation(v, norm);
    rp.tn = tn;
    rp.vel = vel;
    transform_t::forward(buf, rp.axis.data());

    return rp;
}