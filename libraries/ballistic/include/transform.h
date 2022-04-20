#pragma once
#include <structures.h>

#include <static_matrix.h>

namespace ball {
	/// <summary>
	/// Conversion from orthogonal coordinate system to spherical
	/// </summary>
	/// <param name="vec"> - a vector (x,y,z)</param>
	/// <returns>a vector (radius, latitude or inclination, longitude or ascension)</returns>
	math::vec3 ort_to_sph(const math::vec3& vec);
	/// <summary>
	/// Conversion the vector from spherical coordinate system to orthogonal
	/// </summary>
	/// <param name="vec"> - a vector  (radius, latitude or inclination, longitude or ascension)</param>
	/// <returns>a vector (x,y,z)</returns>
	math::vec3 sph_to_ort(const math::vec3& vec);
	/// <summary>
	/// Conversion from GCS to ACS
	/// </summary>
	/// <param name="vec"> - a vector (x,y,z)</param>
	/// <param name="t"> - sidereal time</param>
	/// <returns></returns>
	math::vec3 ACS_to_GCS(const math::vec3& vec, double sidereal_time);
	/// <summary>
	/// Conversion from GCS to ACS
	/// </summary>
	/// <param name="vec"> - a vector (x,y,z)</param>
	/// <param name="t"> - sidereal time</param>
	/// <returns></returns>
	math::vec3 GCS_to_ACS(const math::vec3& vec, double sidereal_time);
	/// <summary>
	/// conversion from ecliptic coordinate system to absolute
	/// </summary>
	/// <param name="vec"> - a vector (x, y, z)</param>
	/// <param name="e"> - ecliptic inclination</param>
	/// <returns>a vector (x, y, z) in ACS</returns>
	math::vec3 ECS_to_ACS(const math::vec3& vec, double ecl_incl);
	/// <summary>
	/// conversion from absolute coordinate system to ecliptic
	/// </summary>
	/// <param name="vec"> - a vector (x, y, z)</param>
	/// <param name="e"> - ecliptic inclination</param>
	/// <returns>a vector (x, y, z) in ECS</returns>
	math::vec3 ACS_to_ECS(const math::vec3& vec, double ecl_incl);
	/// <summary>
	/// Converts vector from absolute coordinate system to greenwich.
	/// </summary>
	/// <param name="vec">6d vector that stores position and velocity</param>
	/// <param name="sidereal_time">sidereal time</param>
	/// <param name="rotational_vel">angular velocity of rotation of the Earth</param>
	math::vec<6> ACS_to_GCS(const math::vec<6>& vec, double sidereal_time, double rotational_vel);
	/// <summary>
	/// Converts vector from greenwich coordinate system to absolute.
	/// </summary>
	/// <param name="vec">6d vector that stores position and velocity</param>
	/// <param name="sidereal_time">sidereal time</param>
	/// <param name="rotational_vel">angular velocity of rotation of the Earth</param>
	math::vec<6> GCS_to_ACS(const math::vec<6>& vec, double sidereal_time, double rotational_vel);
	/// <summary>
	/// Computes the matrix of transform from greenwich to orbital coordinate system.
	/// </summary>
	/// <param name="r">a reference position vector</param>
	/// <param name="v">a reference velocity vector</param>
	/// <returns>matrix 3x3</returns>
	math::mat3x3 GCS_to_OCS(math::vec3 r, math::vec3 v);
	/// <summary>
	/// Computes the matrix of transform from orbital coordinate system to greenwich.
	/// </summary>
	/// <param name="r">a reference position vector</param>
	/// <param name="v">a reference velocity vector</param>
	/// <returns>matrix 3x3</returns>
	math::mat3x3 OCS_to_GCS(math::vec3 r, math::vec3 v);

	/// <summary>
	/// calculating orbital parameters from position and velocity
	/// </summary>
	/// <param name="pos">is a position vector in absolute cartesian coordinate system</param>
	/// <param name="vel">is a velocity vector in absolute cartesian coordinate system</param>
	/// <param name="mu">is a gravitational parameter</param>
	/// <returns>a struct of orbital parameters</returns>
	orbital_params orbparams_from_motionvec(const math::vec3& pos, const math::vec3& vel, double mu);
}