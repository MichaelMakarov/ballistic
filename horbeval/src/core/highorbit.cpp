#include <highorbit.h>
#include <arithmetics.h>
#include <assertion.h>

basic_motion_model::basic_motion_model(size_t harmonics)
{
	_rad = EGM96::rad;
	_fl = EGM96::flat;
	_w = EGM96::angv;
	_gpt = geopotential(EGM96{}, harmonics);
}

vec6 basic_motion_model::acceleration(const vec6& v, const time_h& t)
{
    double w2 = sqr(_w);	
    double h = height_above_ellipsoid(v.data(), _rad, _fl);
    interval i{ minh, maxh };
    ASSERT(
        i(h),
        format(
            "в t = % h = % вышла за пределы ограничений % - %",
            t, h, minh, maxh
        )
    );

    double a[3];
	_gpt.acceleration(v.data(), a);

	return {
		v[3], v[4], v[5],
		a[0] + w2 * v[0] + 2 * _w * v[4],
		a[1] + w2 * v[1] - 2 * _w * v[3],
		a[2]
	};
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
bool point_inside_cone(const vec3& p, double apex, double r)
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
double eclipse(vec3 p, const vec3& sun, double r)
{
    constexpr double res[]{ 1, 0.5, 0 };
    // матрица перехода к координатам, где ось х направлена на Солнце
    auto trmx = make_transform(sun);
    p = trmx * p;
    double dist = length(sun);
    double coef = solar_model::rad() / r;
    // условие полутени
    int index = point_inside_cone(p, dist / (coef + 1), r);
    // условие тени
    index    += point_inside_cone(p, -dist / (coef - 1), r);
    return res[index];
}
/**
 * @brief Вычисление ускорения сообщаемого давлением солнечного света
 * 
 * @param v вектор центра масс КА
 * @param q кватернион поворота
 * @param sun положение Солнца
 * @param r радиус Земли
 * @param obj модель КА
 * @return вектор ускорения
 */
vec3 solar_pressure(const vec3& v, const quaternion& q, const vec3& sun, double r, const object_model* obj)
{
    vec3 ac;
    double ecl = -eclipse(v, sun, r);
    if (ecl < 0) {
        auto sunsat = sun - v;
        ecl *= solar_model::pressure() * sqr(solar_model::AU()) / obj->mass;
        ecl /= sqr(sunsat);
        sunsat = rotate(sunsat, conj(q));
        normalize(sunsat);
        for (const auto& face : obj->surface) {
            double cos_angle = face.norm * sunsat;
            if (cos_angle > 0) {
                auto force = (1 - face.refl) * sunsat + (2 * face.refl * cos_angle) * face.norm;
                force *= cos_angle * face.square;
                ac += force;
            }
        }
        ac *= ecl;
    }
    return rotate(ac, q);
}

extended_motion_model::extended_motion_model(
    size_t harmonics, const rotational_params& rp, const object_model* const obj
) : basic_motion_model(harmonics)
{
    _obj = obj;
    _rp = rp;
}

#include <transform.h>

vec6 extended_motion_model::acceleration(const vec6& v, const time_h& t)
{
    auto ac = basic_motion_model::acceleration(v, t);

    vec3 sun;
    solar_model::coordinates(t, sun.data());
	double st = sidereal_time_mean(t);
    vec3 p;
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(v.data(), st, p.data());
	
	auto sa = solar_pressure(p, _rp.rotation(t), sun, _rad, _obj);
    double buf[3];
    transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(sa.data(), st, buf);

	ac[3] += buf[0];
	ac[4] += buf[1];
	ac[5] += buf[2];

	return ac;
}

quaternion rotational_params::rotation(time_h t) const
{
    // итоговое вращение есть произведение двух кватернионов q = q(t) * q0
    return mult(make_quaternion(axis, vel * (t - tn)), quat);
}
