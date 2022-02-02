#pragma once
#include <structures.h>

namespace ball {
	/// <summary>
	/// calculating the julian centures since 2000
	/// </summary>
	/// <param name="jd"> - julian date related to mig=dnight</param>
	/// <returns></returns>
	constexpr double jd_to_jc2000(const JD& jd) noexcept {
		return (jd - times::JD2000 * times::SEC_PER_DAY).to_double() / 36525;
	}
	/// <summary>
	/// Computes an acceleration caused by massive point.
	/// </summary>
	/// <param name="px">x coordinate of point to compute at</param>
	/// <param name="py">y coordinate of point to compute at</param>
	/// <param name="pz">z coordinate of point to compute at</param>
	/// <param name="mx">x coordinate of massive point</param>
	/// <param name="my">y coordinate of massive point</param>
	/// <param name="mz">z coordinate of massive point</param>
	/// <param name="mu">a gravitational constant of massive point</param>
	/// <returns>a vector of accelerations (x, y, z)</returns>
	XYZ acceleration_by_masspoint(double px, double py, double pz, double mx, double my, double mz, double mu);

	/// <summary>
	/// Interface describes parameters of the Earth and provides methods for computation.
	/// </summary>
	class solar_model {
		friend double sidereal_time_mean(const JD& jd);
		friend double sidereal_time_true(const JD& jd);
	private:
		XYZ _ortpos;
		RIA _sphpos;
	public:
		/// <summary>
		/// Computes solar position.
		/// </summary>
		/// <param name="jd">corresponding julian date</param>
		/// <returns>vector(radius, inclination, ascension)</returns>
		static RIA position(const JD& jd);

		/// <summary>
		/// Gravitational parameter
		/// </summary>
		/// <returns></returns>
		static constexpr inline double mu() { return 1.327124400189e20; }
		/// <summary>
		/// Equatorial radius
		/// </summary>
		/// <returns>meters</returns>
		static constexpr inline double rad() { return 696340e3; }
		/// <summary>
		/// Average distance from the Earth
		/// </summary>
		/// <returns></returns>
		static constexpr inline double AU() { return 149597870700.0; }
		/// <summary>
		/// Average solar flux in the vicinity of the Earth (W * r^-2)
		/// </summary>
		/// <returns></returns>
		static constexpr inline double solar_flux() { return 1367.0; }
		/// <summary>
		/// Average solar pressure in the vicinity of the Earth (N * r^-2)
		/// </summary>
		/// <returns></returns>
		static constexpr inline double solar_pressure() { return 4.56e-6; }
	private:
		constexpr static double ecl_mean_anomaly(double jc) {
			return 6.24003594 + (628.30195602 - (2.7974e-6 + 5.82e-8 * jc) * jc) * jc;
		}
		constexpr static double ecliptic_mean_incl(double jc) {
			return 0.4090928042 - (0.2269655e-3 + (0.29e-8 - 0.88e-8 * jc) * jc) * jc;
		}
		/// difference between lunar and solar ecliptic longitudes
		constexpr static double ecl_delta_lslong(double jc) {
			return 5.19846951 + (7771.37714617 - (3.34085e-5 - 9.21e-8 * jc) * jc) * jc;
		}
	};
	
	/// <summary>
	/// Interface describes parameters of the Earth and provides methods for computation.
	/// </summary>
	class lunar_model {
		friend double sidereal_time_mean(const JD& jd);
		friend double sidereal_time_true(const JD& jd);
	public:
		/// <summary>
		/// Copmutes lunar position.
		/// </summary>
		/// <param name="jd">time</param>
		/// <returns>vector (radius, inclination, ascension)</returns>
		static RIA position(const JD& jd);
		
		/// <summary>
		/// Gravitational parameter
		/// </summary>
		/// <returns></returns>
		static constexpr double mu() { return 4.90486959e12; }
	private:
		constexpr double ecl_mean_anomaly(double jc) {
			return 2.355548393 + (8328.69142288 + (1.517952e-1 + 3.103e-7 * jc) * jc) * jc;
		}
		/// ecliptic lunar argument of latitude
		constexpr static double ecl_mean_latarg(double jc) {
			return 1.62790193 + (8433.46615831 - (6.42717e-5 - 5.33e-8 * jc) * jc) * jc;
		}
		/// ecliptic mean longitude of lunar ascending node
		constexpr static double ecl_mean_ascnode_long(double jc) {
			return 2.182438624 - (33.757045936 - (3.61429e-5 + 3.88e-8 * jc) * jc) * jc;
		}
		/// difference between lunar and solar ecliptic longitudes
		constexpr static double ecl_delta_lslong(double jc) {
			return 5.19846951 + (7771.37714617 - (3.34085e-5 - 9.21e-8 * jc) * jc) * jc;
		}
		
	};
}