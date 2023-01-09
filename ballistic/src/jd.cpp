#include <ball.hpp>
#include <maths.hpp>
#include <chrono>

using namespace math;

/**
 * @brief Юлианская дата для 1 января 2000, 12:00
 *
 */
inline constexpr auto JD2000{2451545.};
/**
 * @brief Юлианская дата для 1 января 1970, 00:00
 *
 */
inline constexpr auto JD1970{2440587.5};
/**
 * @brief Кол-во микросекунд в секунде
 *
 */
inline constexpr time_t microseconds{1'000'000};
/**
 * @brief Кол-во микросекунд в сутках
 *
 */
inline constexpr time_t mcs_per_day{86400 * microseconds};

time_h compute_difference()
{
	auto loc = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto ptr = gmtime(&loc);
	auto gnw = mktime(ptr);
	gnw -= loc;
	if (ptr->tm_isdst)
		gnw -= 3600;
	return {gnw * microseconds};
}

time_t difference()
{
	static time_h diff = compute_difference();
	return diff.mcs;
}
/**
 * @brief Вычисление юлианской даты
 */
double time_to_jd(time_h t)
{
	constexpr double mult{1. / mcs_per_day};
	return (t.mcs - difference()) * mult + JD1970;
}
/**
 * @brief Приведение юлианской даты ко времени
 */
time_h jd_to_time(double jd)
{
	return {static_cast<long long>(jd - JD1970) * mcs_per_day + difference()};
}

double jc2000(time_h t)
{
	return (time_to_jd(t) - JD2000) * (1. / 36525);
}

double daypart(time_h t)
{
	constexpr double mult{1. / mcs_per_day};
	return ((t.mcs - difference()) % mcs_per_day) * mult;
}

double sidereal_time_mean(double jc, double jt)
{
	return fit_round(1.7533685592 + 6.2831853072 * jt + jc * (0.0172027918051 * 36525 + jc * (6.7707139e-6 - 4.50876e-10 * jc)));
}

double sidereal_time_mean(time_h t)
{
	return sidereal_time_mean(jc2000(t), daypart(t));
}

double sidereal_time_true(time_h t)
{
	double jc = jc2000(t);
	// ecliptic inclination
	double e = 0.4090928042 - (0.2269655e-3 + (0.29e-8 - 0.88e-8 * jc) * jc) * jc;
	// solar mean anomaly
	double sa = 6.24003594 + (628.30195602 - (2.7974e-6 + 5.82e-8 * jc) * jc) * jc;
	// difference between lunar and solar longitudes
	double d = 5.19846951 + (7771.37714617 - (3.34085e-5 - 9.21e-8 * jc) * jc) * jc;
	// lunar mean argument of latitude
	double f = 1.62790193 + (8433.46615831 - (6.42717e-5 - 5.33e-8 * jc) * jc) * jc;
	// ecliptic mean longitude of lunar ascending node
	double o = 2.182438624 - (33.757045936 - (3.61429e-5 + 3.88e-8 * jc) * jc) * jc;
	// the Earth's nutation in ascension
	double nut =
		-0.83386e-4 * std::sin(o) + 0.9997e-6 * std::sin(2 * o) + 0.6913e-6 * std::sin(sa) -
		0.63932e-5 * std::sin(2 * (f - d + o)) - 0.11024e-5 * std::sin(2 * (f + o));
	return fit_round(sidereal_time_mean(jc, daypart(t)) + nut * std::cos(e));
}
