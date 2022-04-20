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
	};
	
	/// <summary>
	/// Interface describes parameters of the Earth and provides methods for computation.
	/// </summary>
	class lunar_model {
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
	};

	/// <summary>
	/// Computes solar position in GCS.
	/// </summary>
	/// <param name="jd">a julian date refered to midnight</param>
	/// <returns>3d vector</returns>
	XYZ solar_position(const JD& jd);
	/// <summary>
	/// Computes lunar position in GCS.
	/// </summary>
	/// <param name="jd">a julian date refered to midnight</param>
	/// <returns>3d vector</returns>
	XYZ lunar_position(const JD& jd);

	double sidereal_time_mean(const JD& jd);
}