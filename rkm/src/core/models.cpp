#include <models.hpp>
#include <computation.hpp>
#include <transform.hpp>
#include <formatting.hpp>

basic_model::basic_model(size_t harmonics) : _gpt{harmonics}
{
}

vec6 basic_model::operator()(const vec6 &v, const time_h &t)
{
    double w2 = sqr(egm::angv);
    double h = height_above_ellipsoid(v.data(), egm::rad, egm::flat);
    if (!heights(h))
    {
        throw std::runtime_error(format("При t = % высота h = % вышла за пределы ограничений % - %.", t, h, heights.begin, heights.end));
    }
    vec6 dv;
    for (size_t i{}; i < 3; ++i)
        dv[i] = v[i + 3];
    dv[3] = w2 * v[0] + 2 * egm::angv * v[4];
    dv[4] = w2 * v[1] - 2 * egm::angv * v[3];
    // буффер для ускорения
    double buf[3]{};
    // координаты Солнца
    double sun[3]{};
    // координаты Луны
    double moon[3]{};
    // среднее звёздное время
    double st = sidereal_time(t);

    // вычисляем влияние геопотенциала
    _gpt.acceleration(v.data(), buf);
    for (size_t i{}; i < 3; ++i)
        dv[i + 3] += buf[i];

    // solar_model::coordinates(t, buf);
    // transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, sun);
    // mass_acceleration(v.data(), sun, solar_model::mu(), buf);
    // for (size_t i{}; i < 3; ++i)
    //     dv[i + 3] += buf[i];

    // lunar_model::coordinates(t, buf);
    // transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, st, moon);
    // mass_acceleration(v.data(), moon, lunar_model::mu(), buf);
    // for (size_t i{}; i < 3; ++i)
    //     dv[i + 3] += buf[i];

    return dv;
}

vec42 basic_model1::operator()(vec42 const &v, time_h const &t)
{
    double h = height_above_ellipsoid(v.data(), egm::rad, egm::flat);
    if (!heights(h))
        throw std::runtime_error(format("При t = % высота h = % вышла за пределы ограничений % - %.", t, h, heights.begin, heights.end));
    // вектор производных
    vec42 dv;
    // производные потенциала
    double du[3]{};
    // вторые производные потенциала
    double ddu[3][3]{};
    _gpt.acceleration(v.data() + 36, du, ddu);
    // ускорения модели движения
    for (std::size_t i{}; i < 3; ++i)
        dv[36 + i] = v[36 + 3 + i];
    dv[39] = du[0] + (egm::angv * v[0] + 2 * v[4]) * egm::angv;
    dv[40] = du[1] + (egm::angv * v[1] - 2 * v[3]) * egm::angv;
    dv[41] = du[2];
    for (std::size_t k{}; k < 6; ++k)
    {
        std::size_t n{k * 6};
        for (std::size_t i{}; i < 3; ++i)
        {
            // заполняем скорости
            dv[n + i] = v[n + i + 3];
            // заполняем ускорения
            for (std::size_t j{}; j < 3; ++j)
            {
                dv[n + i + 3] += ddu[i][j] * v[n + j];
            }
        }
        dv[n + 0 + 3] += (egm::angv * v[n + 0] + 2 * v[n + 4]) * egm::angv;
        dv[n + 1 + 3] += (egm::angv * v[n + 1] - 2 * v[n + 3]) * egm::angv;
    }
    return dv;
}

vec3 solar_pressure(vec3 sun, vec3 const &p, double coef);

vec6 extbasic_model::operator()(vec6 const &v, time_h const &t)
{
    auto dv = basic_model::operator()(v, t);
    vec3 sun;
    double buf[3]{};
    solar_model::coordinates(t, buf);
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(buf, sidereal_time(t), sun.data());
    auto ac = solar_pressure(sun, subv<0, 2>(v), _coef);
    for (size_t i{}; i < ac.size(); ++i)
        dv[i + 3] += ac[i];
    return dv;
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
bool point_inside_cone(const vec3 &p, double apex, double r)
{
    // квадрат расстояния от оси х до точки
    double hsqr = sqr(p[1]) + sqr(p[2]);
    // квадрат расстояния от вершины конуса до точки
    double dsqr = hsqr + sqr(p[0] - apex);
    /**
     * сравниваем квадраты синусов углов:
     * если синус угла для точки меньше синуса угла раствора конуса,
     * то точка внутри конуса
     */
    return hsqr / dsqr < sqr(r / apex);
}
/**
 * @brief Условие затенения аппарата Землёй
 *
 * @param p положение аппарата
 * @param sun положение Солнца
 * @param r радиус Земли
 * @return условие затенения (1 - освещение, 0.5 - полутень, 0 - тень)
 */
double eclipse(vec3 p, const vec3 &sun, double r)
{
    return 1.0;
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
 * @brief Вычисление ускорения сообщаемого давлением солнечного света
 *
 * @param v вектор центра масс КА
 * @param q кватернион поворота
 * @param sun положение Солнца
 * @param obj модель КА
 * @return вектор ускорения
 */
vec3 solar_pressure(const vec3 &v, const quaternion &q, const vec3 &sun, const object_model &obj)
{
    vec3 ac;
    double ecl = -eclipse(v, sun, egm::rad);
    if (ecl < 0)
    {
        auto sunsat = sun - v;
        ecl *= solar_model::pressure() * sqr(solar_model::AU()) / obj.mass;
        ecl /= sqr(sunsat);
        sunsat.normalize();
        for (auto &face : obj.surface)
        {
            auto norm = q.rotate(face.norm);
            double cos_angle = norm * sunsat;
            if (cos_angle > 0)
            {
                auto force = (1 - face.refl) * sunsat + (2 * face.refl * cos_angle) * norm;
                force *= cos_angle * face.square;
                ac += force;
            }
        }
        ac *= ecl;
    }
    return ac;
}

extended_model::extended_model(size_t harmonics, const rotator &rot, const object_model &obj)
    : basic_model(harmonics), _obj{obj}, _rot{rot}
{
}

vec6 extended_model::operator()(const vec6 &v, const time_h &t)
{
    auto ac = basic_model::operator()(v, t);

    vec3 sun;
    solar_model::coordinates(t, sun.data());
    double st = sidereal_time(t);
    vec3 p;
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(v.data(), st, p.data());

    auto sa = solar_pressure(p, _rot(t), sun, _obj);
    double buf[3];
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(sa.data(), st, buf);

    ac[3] += buf[0];
    ac[4] += buf[1];
    ac[5] += buf[2];

    return ac;
}

vec3 solar_pressure(vec3 sun, vec3 const &p, double coef)
{
    vec3 ac;
    double ecl = -eclipse(p, sun, egm::rad);
    if (ecl != 0)
    {
        sun -= p;
        double dist_sqr = sqr(sun);
        sun /= std::sqrt(dist_sqr);
        ecl *= solar_model::pressure() * sqr(solar_model::AU()) * coef / dist_sqr;
        ac = ecl * sun;
    }
    return ac;
}