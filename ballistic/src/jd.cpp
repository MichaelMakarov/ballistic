#include <ball.hpp>
#include <ctime>
#include <cmath>

/**
 * @brief Юлианская дата для 1 января 2000, 12:00
 *
 */
inline constexpr auto jd2000{2451545};
/**
 * @brief Юлианская дата для 1 января 1970, 00:00
 *
 */
inline constexpr auto jd1970{2440587.5};
/**
 * @brief Кол-во миллисекунд в сутках
 *
 */
constexpr int64_t ms_per_day{86'400'000};

/**
 * @brief Вычисление юлианской даты.
 *
 * @param t время прошедшее с 1970 года (мс)
 * @return double
 */
double time_to_jd(int64_t t)
{
	constexpr double mult{1. / ms_per_day};
	return t * mult + jd1970;
}
/**
 * @brief Приведение юлианской даты ко времени
 *
 */
int64_t jd_to_time(double jd)
{
	return static_cast<int64_t>(jd - jd1970) * ms_per_day;
}
/**
 * @brief Вычисление кол-ва юлианских столетий по юлианской дате от эпохи J2000.
 *
 * @param jd юлианская дата
 * @return double
 */
double jc2000(double jd)
{
	return (jd - jd2000) * (1. / 36525);
}
/**
 * @brief Разбиение юлианской даты.
 *
 * @param jd исходная юлианская дата
 * @param jd_m юлианская дата на полночь
 * @param frac доля суток
 */
void frac_jd(double jd, double &jd_m, double &frac)
{
	frac = std::modf(jd, &jd_m);
	// проверка на время относительно полуночи
	if (frac >= 0.5)
	{
		frac -= 0.5;
		jd_m += 0.5;
	}
	else
	{
		frac += 0.5;
		jd_m -= 0.5;
	}
}
#include <maths.hpp>
/**
 * @brief Вычисление среднего звёздного времени.
 *
 * @param jc юлианские столетия от эпохи J2000
 * @param jt доля суток
 * @return double
 */
double sidereal_time_mean(double jc, double jt)
{
	return 1.7533685592 + math::pi * 2 /*egm::angv * 864008*/ * jt + jc * (0.0172027918051 * 36525 + jc * (6.77071394e-6 - 4.50876723e-10 * jc));
}

/**
 * @brief Вычисление нутации в прямом восхождении.
 *
 * @param jc юлианские столетия от эпохи j2000
 * @return double
 */
double nutation(double jc)
{
	// средний наклон эклиптики к экватору
	double e = 0.4090928042 - (0.2269655e-3 + (0.29e-8 - 0.88e-8 * jc) * jc) * jc;
	// средняя аномалия Солнца
	double sa = 6.24003594 + (628.30195602 - (2.7974e-6 + 5.82e-8 * jc) * jc) * jc;
	// разность долготы между Солнцем и Луной
	double d = 5.19846951 + (7771.37714617 - (3.34085e-5 - 9.21e-8 * jc) * jc) * jc;
	// аргумент широты Луны
	double f = 1.62790193 + (8433.46615831 - (6.42717e-5 - 5.33e-8 * jc) * jc) * jc;
	// долгота восходящего узла Луны на эклиптике
	double o = 2.182438624 - (33.757045936 - (3.61429e-5 + 3.88e-8 * jc) * jc) * jc;
	// нутация в долготе
	double n_psi =
		-0.83386e-4 * std::sin(o) + 0.9997e-6 * std::sin(2 * o) + 0.6913e-6 * std::sin(sa) -
		0.63932e-5 * std::sin(2 * (f - d + o)) - 0.11024e-5 * std::sin(2 * (f + o));
	// нутация в наклоне
	double n_eps = 0.44615e-4 * std::cos(o) + 0.27809e-5 * std::cos(2 * (f - d + o)) + 0.474e-6 * std::cos(2 * (f + o));
	// нутация в прямом восхождении
	double n_a = n_psi * std::cos(e + n_eps);
	return n_a;
}
/**
 * @brief Вычисление истинного звёздного времени.
 *
 * @param t момент времени в секундах от 1970 г
 * @return double
 */
double sidereal_time_true(double jc, double jt)
{
	// среднее звёздное время + нутация
	return sidereal_time_mean(jc, jt) + nutation(jc);
}

double sidereal_time(int64_t t)
{
	// юлианская дата
	double jd{};
	// доля суток
	double jt{};
	frac_jd(time_to_jd(t), jd, jt);
	// юлианские столетия
	double jc = jc2000(jd + jt);

	return sidereal_time_true(jc, jt);
}