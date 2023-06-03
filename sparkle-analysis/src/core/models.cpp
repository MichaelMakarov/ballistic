#include <models.hpp>
#include <transform.hpp>
#include <times.hpp>
#include <format>
#include <chrono>

void test()
{
}

auto rotational_acceleration(double const in[6])
{
    double constexpr w = egm::angv;
    math::vec3 out;
    out[0] = w * (w * in[0] + 2 * in[4]);
    out[1] = w * (w * in[1] - 2 * in[3]);
    out[2] = 0;
    return out;
}

auto lunar_acceleration(double const in[3], time_t t, double st)
{
    double abs[3], grw[3];
    lunar_model::coordinates(t, abs);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(abs, st, grw);
    math::vec3 out;
    mass_acceleration(in, grw, lunar_model::mu(), out.data());
    return out;
}

auto solar_acceleration(double const in[3], time_t t, double st)
{
    double abs[3], grw[3];
    solar_model::coordinates(t, abs);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(abs, st, grw);
    math::vec3 out;
    mass_acceleration(in, grw, solar_model::mu(), out.data());
    return out;
}

auto gpt_acceleration(double const in[3], geopotential &gpt)
{
    math::vec3 out;
    gpt.acceleration(in, out.data());
    return out;
}

motion_model::motion_model(size_t harmonics) : _gpt{harmonics}
{
}

math::vec6 motion_model::operator()(const math::vec6 &v, time_t t)
{
    double w2 = math::sqr(egm::angv);
    double h = height_above_ellipsoid(v.data(), egm::rad, egm::flat);
    if (!heights(h))
    {
        throw std::runtime_error(std::format("t = {} height {} is out of bounds {} - {}.",
                                             time_type(std::chrono::milliseconds(t)),
                                             h, heights.begin, heights.end));
    }
    //  перевод в секунды
    t /= 1000;
    double st = sidereal_time(t);
    auto rotac = rotational_acceleration(v.data());
    auto gptac = gpt_acceleration(v.data(), _gpt);
    auto solac = solar_acceleration(v.data(), t, st);
    auto lunac = lunar_acceleration(v.data(), t, st);
    return math::vec6{
        v[3],
        v[4],
        v[5],
        rotac[0] + gptac[0], // + solac[0] + lunac[0],
        rotac[1] + gptac[1], // + solac[1] + lunac[1],
        rotac[2] + gptac[2], // + solac[2] + lunac[2],
    };
}
