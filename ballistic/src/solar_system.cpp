#include <solar_system.h>
#include <conversion.h>
#include <transform.h>
#include <linalg.h>

namespace ball {

	RIA solar_model::position(const times::juliandate& jd) {
		using namespace math;

		const double t = jd_to_jc2000(jd);	// julian centures since 2000
		// a constant of the aberration
		constexpr double hi{ sec_to_rad(20.49552) };
		constexpr double ac{ 149597870691 };
		// solar average longitude
		double L = sec_to_rad(1009677.85 + (100 * SEC_PER_ROUND + 2771.27 + 1.089 * t) * t);
		// solar perigee average longitude
		double lc = sec_to_rad(1018578.046 + (6190.046 + (1.666 + 0.012 * t) * t) * t);
		// the Earth's orbit eccentricity
		double e = 0.0167086342 - (0.000004203654 + (0.00000012673 + 0.00000000014 * t) * t) * t;
		// average ecliptic inclination
		double eps = sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * t) * t) * t);
		// ecliptic average longitude of lunar ascending node
		double omega = sec_to_rad(450160.280 - (5 * SEC_PER_ROUND + 482890.539 - (7.455 + 0.008 * t) * t) * t);
		// long periodic nutation of the Earth
		double psi = sec_to_rad(-17.1996 * std::sin(omega));
		// solar longitude
		double longitude = L + 2 * e * std::sin(L - lc) + 1.25 * e * e * std::sin(2 * (L - lc));
		double sinl{ std::sin(longitude) };
		double cosl{ std::cos(longitude) };
		double sine{ std::sin(eps) };
		double cose{ std::cos(eps) };

		RIA pos;
		pos[1] = std::atan(sinl * sine / std::sqrt(cosl * cosl + sinl * sinl * cose * cose));
		pos[2] = std::atan(sinl / cosl * cose);

		if (pos[2] < 0.0) {
			if (pos[1] < 0.0) pos[2] += PI2;
			else pos[2] += PI;
		}
		else {
			if (pos[1] < 0.0) pos[2] += PI;
		}
		pos[2] += 0.061165 * psi - hi;
		pos[1] += hi * sine * cosl;
		pos[0] = ac * (1 - e * (std::cos(L - lc) - e * 0.5 * (1 - std::cos(2 * (L - lc)))));

		return pos;
	}

	 RIA lunar_model::position(const times::juliandate& jd) {
		using namespace math;
		double jc = jd_to_jc2000(jd);	// julian centures since 2000
		// radius of Earth's equator
		double r{ 6378136 };
		// mean lunar anomaly
		double la = sec_to_rad((485866.733 + (1325 * SEC_PER_ROUND + 715922.633 + (31.31 + 0.064 * jc) * jc) * jc));
		// solar average anomaly
		double sa = sec_to_rad((1287099.804 + (99 * SEC_PER_ROUND + 1292581.224 - (0.577 + 0.012 * jc) * jc) * jc));
		// average arg of lunar latitude
		double f = sec_to_rad((335778.877 + (1342 * SEC_PER_ROUND + 295263.137 - (13.257 - 0.011 * jc) * jc) * jc));
		// average alongation (a difference between solar and lunar longitudes)
		double d = sec_to_rad((1072261.307 + (1236 * SEC_PER_ROUND + 1105601.328 - (6.891 - 0.019 * jc) * jc) * jc));
		// lunar ecliptic latitude
		double latitude = sec_to_rad(18461.48 * std::sin(f) +
			1010.18 * std::sin(la + f) - 999.69 * std::sin(f - la) -
			623.65 * std::sin(f - 2 * d) + 199.48 * std::sin(f + 2 * d - la) -
			166.57 * std::sin(la + f - 2 * d) +
			117.26 * std::sin(f + 2 * d) + 61.91 * std::sin(2 * la + f) -
			33.35 * std::sin(f - 2 * d - la) - 31.76 * std::sin(f - 2 * la) -
			29.68 * std::sin(sa + f - 2 * d) + 15.125 * std::sin(la + f + 2 * d) -
			15.56 * std::sin(2 * (la - d) + f));
		// lunar ecliptic longitude
		double longitude = sec_to_rad(785939.157 +
			(1336 * SEC_PER_ROUND + 1108372.598 + (5.802 + 0.019 * jc) * jc) * jc +
			22639.5 * std::sin(la) - 4586.42 * std::sin(la - 2 * d) + 2369.9 * std::sin(2 * d) +
			769.01 * std::sin(2 * la) - 668.11 * std::sin(sa) - 411.6 * std::sin(2 * f) -
			211.65 * std::sin(2 * (la - d)) -
			205.96 * std::sin(la + sa - 2 * d) + 191.95 * std::sin(la + 2 * d) -
			165.14 * std::sin(sa - 2 * d) +
			147.69 * std::sin(la - sa) - 125.15 * std::sin(d) - 109.66 * std::sin(la + sa) -
			55.17 * std::sin(2 * (f - d)) - 45.1 * std::sin(sa + 2 * f) + 39.53 * std::sin(la - 2 * f) -
			38.42 * std::sin(la - 4 * d) + 36.12 * std::sin(3 * la) - 30.77 * std::sin(2 * la - 4 * d) +
			28.47 * std::sin(la - sa - 2 * d) - 24.42 * std::sin(sa + 2 * d) + 18.6 * std::sin(la - d) +
			18.02 * std::sin(sa - d));
		longitude = fit_to_round(longitude);
		// lunar paralax
		double paralax = sec_to_rad(3422.7 + 186.539 * std::cos(la) + 34.311 * std::cos(la - 2 * d) +
			28.233 * std::cos(2 * d) + 10.165 * std::cos(2 * la) + 3.086 * std::cos(la + 2 * d) +
			1.92 * std::cos(sa - 2 * d) + 1.445 * std::cos(la + sa - 2 * d) + 1.154 * std::cos(la - sa) -
			0.975 * std::cos(d) - 0.95 * std::cos(la + sa) - 0.713 * std::cos(la - 2 * f) +
			0.6215 * std::cos(3 * la) + 0.601 * std::cos(la - 4 * d));
		double radius = r / paralax;
		return ECS_to_ACS(
			sph_to_ort({ radius, latitude, longitude }),
			sec_to_rad(84381.448 - (46.815 + (0.00059 - 0.001813 * jc) * jc) * jc));
	}

	
	XYZ acceleration_by_masspoint(const XYZ& point, const XYZ& pos, double mu) {
		auto diff = pos - point;
		return mu * (diff / math::cube(length(diff)) - pos / math::cube(length(pos)));
	}

	XYZ acceleration_by_masspoint(double px, double py, double pz, double mx, double my, double mz, double mu) {
		XYZ diff{ mx - px, my - py, mz - pz };
		XYZ mass{ mx, my, mz };
		return mu * (diff / math::cube(length(diff)) - mass / math::cube(length(mass)));
	}

}