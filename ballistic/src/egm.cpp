#include <egm.h>
#include <conversion.h>
#include <arithmetics.h>
#include <solar_system.h>


namespace ball {

	std::vector<potential_harmonic> egm_constants<egm_type::EGM08>::harmonics{};
	std::vector<potential_harmonic> egm_constants<egm_type::EGM96>::harmonics{};
	std::vector<potential_harmonic> egm_constants<egm_type::JGM3>::harmonics{};


	double sidereal_time_true(const JD& jd) {
		double jc = jd_to_jc2000(jd);
		// ecliptic inclination
		double e = solar_model::ecliptic_mean_incl(jc);
		//solar mean anomaly
		double sa = solar_model::ecl_mean_anomaly(jc);
		// difference between lunar and solar longitudes
		double d = solar_model::ecl_delta_lslong(jc);
		// lunar mean argument of latitude
		double f = lunar_model::ecl_mean_latarg(jc);
		// ecliptic mean longitude of lunar ascending node
		double o = lunar_model::ecl_mean_ascnode_long(jc);
		// the Earth's nutation in ascension
		double nut = -0.83386e-4 * std::sin(o) + 0.9997e-6 * std::sin(2 * o) + 0.6913e-6 * std::sin(sa) -
			0.63932e-5 * std::sin(2 * (f - d + o)) - 0.11024e-5 * std::sin(2 * (f + o));
		return math::fit_to_round(sidereal_time_mean(jd) + nut * std::cos(e));
	}

	std::istream& operator>> (std::istream& is, potential_harmonic& ph) {
		return is >> ph.cos >> ph.sin;
	}

	double sidereal_time_mean(const JD& jd) {
		double jc = jd_to_jc2000(jd);
		return math::fit_to_round(1.7533685592 + 6.2831853072 * jd.day + jc * (0.0172027918051 * 36525 + jc * (6.7707139e-6 - 4.50876e-10 * jc)));
	}

	double height_above_ellipsoid(double x, double y, double z, double rad, double flat) {
		double dist = std::sqrt(x * x + y * y + z * z);
		return dist - rad * (1 - flat * math::sqr(z / dist));
	}
}