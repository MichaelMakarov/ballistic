#include <model.hpp>
#include <transform.hpp>
#include <atmosphere.hpp>
#include <spaceweather.hpp>
#include <format>

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

/**
 * @brief Вычисление тормозного ускорения от атмосферы.
 *
 * @param v векторо скорости
 * @param density плотность атмосферы
 * @param s баллистический к-т
 */
void atmforce(math::vec3 const &v, double density, double s, double *out)
{
    double k = v.length() * density * s;
    for (std::size_t i{}; i < 3; ++i)
    {
        out[i] = k * v[i];
    }
}

void atmforce(math::vec3 const &v, double density,
              geometry const *geometries, std::size_t count,
              double cx, math::quaternion const &q,
              double *out)
{
    math::vec3 l{v};
    l.normalize();
    // площадь отражающей поверхности
    double s{};
    for (std::size_t i{}; i < count; ++i)
    {
        auto &geom = geometries[i];
        auto n = q.rotate(geom.n);
        double mul = n * l;
        if (mul > 0)
        {
            double ds = geom.s * mul;
            s += ds;
        }
    }
    atmforce(v, density, cx * s, out);
}

/**
 * @brief Проверка, находится ли точка внутри конуса
 *
 * @param p координаты точки (x, y, z)
 * @param apex вершина конуса (x, y, z)
 * @param r радиус Земли
 * @return true если точка внутри конуса
 * @return false если точка за пределами конуса
 */
bool point_inside_cone(const math::vec3 &p, double apex, double r)
{
    // квадрат расстояния от оси х до точки
    double hsqr = math::sqr(p[1]) + math::sqr(p[2]);
    // квадрат расстояния от вершины конуса до точки
    double dsqr = hsqr + math::sqr(p[0] - apex);
    /**
     * сравниваем квадраты синусов углов:
     * если синус угла для точки меньше синуса угла раствора конуса,
     * то точка внутри конуса
     */
    return hsqr / dsqr < math::sqr(r / apex);
}
/**
 * @brief Условие затенения аппарата Землёй
 *
 * @param p положение аппарата
 * @param sun положение Солнца
 * @param r радиус Земли
 * @return условие затенения (1 - освещение, 0.5 - полутень, 0 - тень)
 */
double eclipse(math::vec3 p, const math::vec3 &sun, double r)
{
    constexpr double res[]{1, 0.5, 0};
    // матрица перехода к координатам, где ось х направлена на Солнце
    auto trmx = make_transform(sun);
    p = trmx * p;
    double dist = sun.length();
    double coef = solar_model::rad() / r;
    // условие полутени
    int index = point_inside_cone(p, dist / (coef + 1), r);
    // условие тени
    index += point_inside_cone(p, -dist / (coef - 1), r);
    return res[index];
}
/**
 * @brief Вычисление ускорения от давления солнечного света.
 *
 * @param sun координаты Солнца в ГСК
 * @param p координаты точки в ГСК
 * @param coef (1 + к-т отражения) * площадь отражающей поверхности
 * @param ac ускорение
 */
void lightforce(math::vec3 sun, math::vec3 const &p, double coef, double ac[3])
{
    double ecl = -eclipse(p, sun, egm::rad);
    if (ecl != 0)
    {
        sun -= p;
        double dist_sqr = sqr(sun);
        sun /= std::sqrt(dist_sqr);
        ecl *= solar_model::pressure() * math::sqr(solar_model::AU()) * coef / dist_sqr;
        sun *= ecl;
        for (std::size_t i{}; i < 3; ++i)
            ac[i] = sun[i];
    }
    else
    {
        for (std::size_t i{}; i < 3; ++i)
            ac[i] = 0;
    }
}

double compute_square(math::vec3 const &v, geometry const *geometries, std::size_t count, math::quaternion const &q)
{
    double square{};
    for (std::size_t i{}; i < count; ++i)
    {
        auto &geom = geometries[i];
        auto n = q.rotate(geom.n);
        double prod = n * v;
        if (prod > 0)
        {
            prod /= v.length();
            double s = geom.s * prod;
            square += s;
        }
    }
    return square;
}

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
    auto atmforce(double const v[3], double h, time_t t) const
    {
        auto w = get_spaceweather(t);
        double r = std::sqrt(math::sqr(_coords[0]) + math::sqr(_coords[1]) + math::sqr(_coords[2]));
        double lg = std::atan2(_coords[1], _coords[0]);
        double incl = std::asin(_coords[2] / r);
        double dens = atmosphere2004(v, h, t, lg, incl, w.f10_7, w.f81, w.kp);
        double vel = std::sqrt(math::sqr(v[3]) + math::sqr(v[4]) + math::sqr(v[5]));
        double k = vel * dens;
        math::vec3 a;
        for (std::size_t i{}; i < 3; ++i)
        {
            a[i] = k * v[3 + i];
        }
        return a;
    }
    auto diffatmforce(double const v[3], double h, time_t t) const
    {
        auto w = get_spaceweather(t);
        double r = std::sqrt(math::sqr(_coords[0]) + math::sqr(_coords[1]) + math::sqr(_coords[2]));
        double lg = std::atan2(_coords[1], _coords[0]);
        double incl = std::asin(_coords[2] / r);
        double dens = atmosphere2004(v, h, t, lg, incl, w.f10_7, w.f81, w.kp);
        double vel = std::sqrt(math::sqr(v[3]) + math::sqr(v[4]) + math::sqr(v[5]));
        double kv = dens * vel;
        math::vec3 a;
        math::mat3x3 da;
        for (std::size_t i{}; i < 3; ++i)
        {
            a[i] = kv * v[3 + i];
            for (std::size_t j{}; j < 3; ++j)
            {
                da[i][j] = v[3 + i] * v[3 + j] * dens / vel;
            }
            da[i][i] += kv;
        }
        return std::make_tuple(a, da);
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

enum class yeartype
{
    millineum = 1
};

constexpr yeartype yy = static_cast<yeartype>(1);

motion_model::motion_model(std::size_t harmonics, double sball)
    : _gpt{harmonics},
      _sb{sball}
{
}

// motion_model::motion_model(std::size_t harmonics, double sball,
//                            std::vector<geometry> const &geometries,
//                            rotator const &rot)
//     : _gpt{harmonics},
//       _sball{sball},
//       _geometries{geometries},
//       _rotator{rot}
// {
// }

double motion_model::check_height(double const v[3], std::time_t t) const
{
    double h = std::sqrt(math::sqr(v[0]) + math::sqr(v[1]) + math::sqr(v[2])) - egm::rad;
    if (!heights(h))
    {
        throw std::invalid_argument(std::format("Height {} at {} is out of bounds {} - {}.",
                                                h, std::chrono::system_clock::from_time_t(t),
                                                heights.begin, heights.end));
    }
    return h;
}

math::vec6 motion_model::operator()(math::vec6 const &v, time_t t)
{
    double h = check_height(v.data(), t);
    double st = sidereal_time(t); // звёздное время
    sun s{t, st};
    moon m{t, st};
    auto rotac = rotforce(v.data());
    auto gptac = gptforce(v.data(), _gpt);
    auto lunac = m.gptforce(v.data());
    auto solac = s.gptforce(v.data());
    auto atmac = s.atmforce(v.data(), h, t);
    for (std::size_t i{}; i < 3; ++i)
        atmac[i] *= _sb;
    return {
        v[3],
        v[4],
        v[5],
        rotac[0] + gptac[0] + lunac[0] + solac[0] + atmac[0],
        rotac[1] + gptac[1] + lunac[1] + solac[1] + atmac[1],
        rotac[2] + gptac[2] + lunac[2] + solac[2] + atmac[2],
    };
}

vec55 motion_model::operator()(vec55 const &v, time_t t)
{
    double h = check_height(v.data(), t);
    double st = sidereal_time(t);
    sun s{t, st};
    moon m{t, st};
    double gptac[3], gptmx[3][3];
    _gpt.ddiffbyxyz(v.data(), gptac, gptmx);
    auto solac_ = s.gptforce(v.data());
    auto lunac_ = m.gptforce(v.data());
    auto [solac, solmx] = s.diffgptforce(v.data());
    auto [lunac, lunmx] = m.diffgptforce(v.data());
    auto [atmac, atmmx] = s.diffatmforce(v.data(), h, t);
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
        out[3 + i] += gptac[i] + solac[i] + lunac[i] + atmac[i] * _sb;
    }
    // заносим ускорения от вариаций геопотенциала
    for (std::size_t i{}; i < vdim; ++i)
    {
        std::size_t index{6 + i * vdim};
        for (std::size_t j{}; j < 3; ++j)
        {
            for (std::size_t k{}; k < 3; ++k)
            {
                out[index + 3 + j] += (gptmx[j][k] + solmx[j][k] + lunmx[j][k] + atmmx[j][k] * _sb) * v[index + k];
            }
            out[index + 3 + j] += atmac[j] * v[index + 6];
        }
    }
    return out;
}