#include <models.hpp>
#include <transform.hpp>
#include <times.hpp>
#include <format>
#include <chrono>

class sun
{
    double _coords[3];

public:
    sun(time_t t, double st)
    {
        double buf[3];
        solar_model::coordinates(t, buf);
        transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, _coords);
    }
    math::vec3 gptforce(double const in[3]) const
    {
        math::vec3 out;
        massforce(in, _coords, solar_model::mu(), out.data());
        return out;
    }
    auto diffgptforce(double const v[3]) const
    {
        math::vec3 a;
        math::mat3x3 da;
        massforce(v, _coords, solar_model::mu(), a.data(), da.data());
        return std::make_pair(a, da);
    }
    math::vec3 lightforce(double s) const
    {
        constexpr double coef = -solar_model::pressure() * math::sqr(solar_model::AU());
        math::vec3 out;
        double _r3{};
        for (double e : _coords)
        {
            _r3 += math::sqr(e);
        }
        _r3 = 1 / std::pow(_r3, 1.5);
        for (size_t i{}; i < 3; ++i)
        {
            out[i] = coef * s * _r3 * _coords[i];
        }
        return out;
    }
};

class moon
{
    double _coords[3];

public:
    moon(time_t t, double st)
    {
        double buf[3];
        lunar_model::coordinates(t, buf);
        transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, _coords);
    }
    math::vec3 gptforce(double const in[3]) const
    {
        math::vec3 out;
        massforce(in, _coords, lunar_model::mu(), out.data());
        return out;
    }
    auto diffgptforce(double const v[3]) const
    {
        math::vec3 a;
        math::mat3x3 da;
        massforce(v, _coords, lunar_model::mu(), a.data(), da.data());
        return std::make_pair(a, da);
    }
};

auto rotforce(double const v[6])
{
    double constexpr w = egm::angv;
    math::vec3 a;
    a[0] = w * (w * v[0] + 2 * v[4]);
    a[1] = w * (w * v[1] - 2 * v[3]);
    a[2] = 0;
    return a;
}

auto gptforce(double const in[3], geopotential &gpt)
{
    math::vec3 out;
    gpt.diffbyxyz(in, out.data());
    return out;
}

auto diffgptforce(double const xyz[3])
{
    math::mat3x3 a;
    double r2 = math::sqr(xyz[0]) + math::sqr(xyz[1]) + math::sqr(xyz[2]);
    double mult = egm::mu / (r2 * std::sqrt(r2));
    // вариации, связанные с потенциалом
    a[0][0] = 3 * math::sqr(xyz[0]) / r2 - 1;
    a[1][1] = 3 * math::sqr(xyz[1]) / r2 - 1;
    a[2][2] = 3 * math::sqr(xyz[2]) / r2 - 1;
    a[0][1] = a[1][0] = 3 * xyz[0] * xyz[1] / r2;
    a[0][2] = a[2][0] = 3 * xyz[0] * xyz[2] / r2;
    a[1][2] = a[2][1] = 3 * xyz[1] * xyz[2] / r2;
    a *= mult;
    return a;
}

motion_model::motion_model(size_t harmonics, double s) : _gpt{harmonics}, _s{s}
{
}

void motion_model::verify_height(double const v[3], time_t t)
{
    double h = height_above_ellipsoid(v, egm::rad, egm::flat);
    if (!heights(h))
    {
        throw std::runtime_error(std::format("t = {} height {} is out of bounds {} - {}.",
                                             time_type(std::chrono::milliseconds(t)),
                                             h, heights.begin, heights.end));
    }
}

math::vec6 motion_model::operator()(const math::vec6 &v, time_t t)
{
    verify_height(v.data(), t);
    //  перевод в секунды
    t /= 1000;
    double st = sidereal_time(t);
    sun s{t, st};
    moon m{t, st};
    auto rotac = rotforce(v.data());
    auto gptac = gptforce(v.data(), _gpt);
    auto solac = s.gptforce(v.data());
    auto lunac = m.gptforce(v.data());
    auto preac = s.lightforce(_s);
    return math::vec6{
        v[3],
        v[4],
        v[5],
        rotac[0] + gptac[0] + solac[0] + lunac[0] + preac[0],
        rotac[1] + gptac[1] + solac[1] + lunac[1] + preac[1],
        rotac[2] + gptac[2] + solac[2] + lunac[2] + preac[2],
    };
}

vec55 motion_model::operator()(vec55 const &v, time_t t)
{
    verify_height(v.data(), t);
    t /= 1000;
    double st = sidereal_time(t);
    sun s{t, st};
    moon m{t, st};
    double gptac[3], gptmx[3][3];
    _gpt.ddiffbyxyz(v.data(), gptac, gptmx);
    auto solac_ = s.gptforce(v.data());
    auto lunac_ = m.gptforce(v.data());
    auto [solac, solmx] = s.diffgptforce(v.data());
    auto [lunac, lunmx] = m.diffgptforce(v.data());
    auto ligac = s.lightforce(_s);
    vec55 out;
    constexpr std::size_t vdim{7};
    for (std::size_t i{}; i <= vdim; ++i)
    {
        std::size_t index = (6 + (i - 1) * vdim) * std::size_t(i > 0);
        // заносим производные координат
        for (std::size_t j{}; j < 3; ++j)
        {
            out[index + j] = v[index + j + 3];
        }
        auto rotac = rotforce(v.data() + index);
        // заносим ускорение от врашения ГСК
        for (std::size_t j{}; j < 3; ++j)
        {
            out[index + j + 3] = rotac[j];
        }
    }
    // заносим ускорения геопотенциала
    for (std::size_t i{}; i < 3; ++i)
    {
        out[3 + i] += gptac[i] + solac[i] + lunac[i] + ligac[i];
    }
    // заносим ускорения от вариаций геопотенциала
    for (std::size_t i{}; i < vdim; ++i)
    {
        std::size_t index{ 6 + i * vdim };
        auto preac = s.lightforce(v[index + 6]);
        for (std::size_t j{}; j < 3; ++j)
        {
            for (std::size_t k{}; k < 3; ++k)
            {
                out[index + 3 + j] += (gptmx[j][k] + solmx[j][k] + lunmx[j][k]) * v[index + k];
            }
            out[index + 3 + j] += preac[j];
        }
    }
    return out;
}