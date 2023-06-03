#include <model.hpp>
#include <transform.hpp>
#include <atmosphere.hpp>
#include <spaceweather.hpp>
#include <format>

/**
 * @brief Ускорения во вращающейся СК
 *
 * @param in параметры движения (x, y, z, vx, vy, vz)
 * @param out усорения (ax, ay, az)
 */
void rotational_acceleration(double const *in, double *out)
{
    double constexpr w = egm::angv;
    out[0] = w * (w * in[0] + 2 * in[4]);
    out[1] = w * (w * in[1] - 2 * in[3]);
    out[2] = 0;
}

/**
 * @brief Вычисление тормозного ускорения от атмосферы.
 *
 * @param v векторо скорости
 * @param density плотность атмосферы
 * @param s баллистический к-т
 */
void atmosphere_deceleration(math::vec3 const &v, double density, double s, double *out)
{
    double k = v.length() * density * s;
    for (std::size_t i{}; i < 3; ++i)
    {
        out[i] = k * v[i];
    }
}

void atmosphere_deceleration(math::vec3 const &v, double density,
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
    atmosphere_deceleration(v, density, cx * s, out);
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
void solar_pressure_acceleration(math::vec3 sun, math::vec3 const &p, double coef, double ac[3])
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

motion_model::motion_model(std::size_t harmonics, double sball,
                           std::vector<geometry> const &geometries,
                           rotator const &rot)
    : _gpt{harmonics},
      _sball{sball},
      _geometries{geometries},
      _rotator{rot}
{
}

math::vec6 motion_model::operator()(math::vec6 const &v, time_t t)
{
    double h = height_above_ellipsoid(v.data(), egm::rad, egm::flat);
    if (!heights(h))
    {
        throw std::invalid_argument(std::format("Height {} is out of bounds {} - {}.", h, heights.begin, heights.end));
    }
    double rot_ac[3]{}; // ускорение из-за вращения СК
    double gpt_ac[3]{}; // ускорение от геопотенциала
    double atm_ac[3]{}; // ускорение от атмосферы
    double sol_ac[3]{}; // ускорение от Солнца
    double lun_ac[3]{}; // ускорение от Луны
    double lig_ac[3]{}; // ускорение от солнечного света
    double buf[3]{};    // буффер
    double moon[3]{};   // координаты Луны
    math::vec3 sun{};   // координаты Солнца

    rotational_acceleration(v.data(), rot_ac);
    _gpt.acceleration(v.data(), gpt_ac);

    double st = sidereal_time(t); // звёздное время

    lunar_model::coordinates(t, buf);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, moon);
    mass_acceleration(v.data(), moon, lunar_model::mu(), lun_ac);

    solar_model::coordinates(t, buf);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, sun.data());
    mass_acceleration(v.data(), sun.data(), solar_model::mu(), sol_ac);

    auto s = get_spaceweather(t);
    double solar_long = std::atan2(sun[1], sun[0]);
    double solar_incl = std::atan(buf[2] / std::sqrt(math::sqr(buf[0]) + math::sqr(buf[1])));
    double density = atmosphere2004(v.data(), h, t, solar_long, solar_incl, s.f10_7, s.f81, s.kp);
    // double density = atmosphere1981(h);
    // atmosphere_deceleration(v.subv<3, 3>(), density, _sball, atm_ac);
    math::vec3 vv;
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(v.subv<3, 3>().data(), st, vv.data());
    atmosphere_deceleration(vv, density, _geometries.data(), _geometries.size(), _sball,
                            _rotator.get_quaternion(t), buf);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, atm_ac);

    return {v[3], v[4], v[5],
            rot_ac[0] + gpt_ac[0] + lun_ac[0] + sol_ac[0] + atm_ac[0] + lig_ac[0],
            rot_ac[1] + gpt_ac[1] + lun_ac[1] + sol_ac[1] + atm_ac[1] + lig_ac[1],
            rot_ac[2] + gpt_ac[2] + lun_ac[2] + sol_ac[2] + atm_ac[2] + lig_ac[2]};
}