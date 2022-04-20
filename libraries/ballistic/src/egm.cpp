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
		double e = 0.4090928042 - (0.2269655e-3 + (0.29e-8 - 0.88e-8 * jc) * jc) * jc;
		//solar mean anomaly
		double sa = 6.24003594 + (628.30195602 - (2.7974e-6 + 5.82e-8 * jc) * jc) * jc;
		// difference between lunar and solar longitudes
		double d = 5.19846951 + (7771.37714617 - (3.34085e-5 - 9.21e-8 * jc) * jc) * jc;
		// lunar mean argument of latitude
		double f = 1.62790193 + (8433.46615831 - (6.42717e-5 - 5.33e-8 * jc) * jc) * jc;
		// ecliptic mean longitude of lunar ascending node
		double o = 2.182438624 - (33.757045936 - (3.61429e-5 + 3.88e-8 * jc) * jc) * jc;
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
		return math::fit_to_round(1.7533685592 + 6.2831853072 * jd.time + jc * (0.0172027918051 * 36525 + jc * (6.7707139e-6 - 4.50876e-10 * jc)));
	}

	double height_above_ellipsoid(double x, double y, double z, double rad, double flat) {
		double dist = std::sqrt(x * x + y * y + z * z);
		return dist - rad * (1 - flat * math::sqr(z / dist));
	}
}