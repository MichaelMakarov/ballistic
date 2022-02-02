#pragma once
#include <static_vector.h>
#include <juliandate.h>

namespace ball {
	/// 6d vector (x, y, z, vx, vy, vz)
	using RV = math::vec<6>;
	/// 3d vector (x, y, z)
	using XYZ = math::vec3;
	/// 3d vector (radius, inclination, right ascension)
	using RIA = math::vec3;
	/// 3d vector (radius, latitude, longitude)
	using RLL = math::vec3;
	/// a julian date
	using JD = times::juliandate;

	/// <summary>
	/// Parameters of motion
	/// </summary>
	template<size_t vdim = 6>
	struct motion_params {
		math::vec<vdim> vec;		// state vector
		JD jd;						// julian date
		size_t loop{};				// loop number
	};

	/// <summary>
	/// Orbital parameters (Keplerian elements)
	/// </summary>
	struct orbital_params {
		double semiaxis;		// semimajor axis
		double eccentricity;	// eccentricity
		double inclination;		// inclination
		double periapsis;		// argument of periapsis
		double ascendnode;		// longitude of the ascending node
		double meananomaly;		// mean anomaly
		double ecanomaly;		// eccentric anomaly
		double trueanomaly;		// true anomaly
		double latitudearg;		// longitude argument
	};

	template<size_t dim>
	std::ostream& operator<< (std::ostream& os, const motion_params<dim>& params) {
		return os << times::make_datetime(params.jd) << " " << params.vec << " loop = " << params.loop;
	}
}