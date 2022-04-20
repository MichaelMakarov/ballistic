#pragma once
#include <geopotential.h>
#include <structures.h>

#include <quaternion.h>

/// vector (rx, ry, rz, vx, vy, vz, q0, q1, q2, q3, wx, wy, wz)
using RVE = math::vec<13>;

/// <summary>
/// Surface parameters
/// </summary>
struct surface_params {
	math::vec3 center;	// a position of surface center
	math::vec3 norm;	// a normal vector
	double refl{};		// an optical parameter
	double square{};	// surface square
};

/// <summary>
/// 3d geometry model of object
/// </summary>
struct object_model {
	std::vector<surface_params> surface;	// surface units (every surface unit is characterized by square, optical parameter and center coordinates)
	math::vec3 inertia;						// moments of inertia J = (Jx, Jy, Jz)
	double mass{};							// mass of whole construction of object
};

class basic_motion_model {
protected:
	double _w, _fl, _rad;
	ball::geopotential _gpt;
public:
	double minheight{ 1e5 }, maxheight{ 1e8 };
public:
	explicit basic_motion_model(size_t harmonics);

};

/** @brief Parameters of rotation in ACS */
struct rotation_params {
	/** @brief axis of rotation */
	math::vec3 axis;
	/** @brief angular velocity of rotation */
	double vel;
	/** @brief quaternion of initial rotation */
	math::quaternion quat;
	/** @brief start time */
	ball::JD jd;

	math::quaternion rotation(const ball::JD& jd) const;
};

class linear_motion_model : public basic_motion_model {
public:
	using basic_motion_model::basic_motion_model;

	ball::RV acceleration(const ball::RV& vc, const ball::JD& jd);
};

/** @brief Extended model of motion (influence of rotation is considered) */
class extended_motion_model : public linear_motion_model {
	const object_model* _obj;
	rotation_params _rp;
public:
	extended_motion_model(
		size_t harmonics,
		const rotation_params& rp,
		const object_model* obj
	);

	ball::RV acceleration(const ball::RV& vc, const ball::JD& jd);
};

class angular_motion_model : public basic_motion_model {
	const object_model* _obj;
	ball::XYZ _av;		// coefficients of equation of rotation (before angular velocities)
	ball::XYZ _bv;		// coefficients of equation of rotation (before torques)
public:
	explicit angular_motion_model(
		size_t harmonics,
		const object_model* const obj
	);

	RVE acceleration(
		const RVE& vc, 
		const ball::JD& jd
	);
};

/// <summary>
/// Verifying eclipse condition.
/// </summary>
/// <param name="sun">solar position in GCS</param>
/// <param name="point">point to verify eclipse condition at (coordinates in GCS)</param>
/// <param name="erad">radius of the Earth</param>
/// <returns>0 - umbra, 0.5 - penumbra, 1 - lighted</returns>
double eclipse_condition(const math::vec3 sun, const math::vec3& point, double erad);
/// <summary>
/// Phase function of the diffuse plate.
/// </summary>
/// <param name="obs">a position of the observer in GCS</param>
/// <param name="sun">a position of the sun in GCS</param>
/// <returns></returns>
double plate_phase_function(const ball::XYZ& obs, const ball::XYZ& sun);
/// <summary>
/// Phase function of the diffuse sphere.
/// </summary>
/// <param name="obs">a position of the observer in GCS</param>
/// <param name="sun">a position of the sun in GCS</param>
/// <param name="sat">a position of the sat in GCS</param>
/// <returns></returns>
double sphere_phase_function(const ball::XYZ& obs, const ball::XYZ& sun, const ball::XYZ& sat);

enum class geometry_type {
	diffuse_plate, 
	diffuse_sphere
};


/// <summary>
/// Computes apparent magnitude.
/// </summary>
/// <param name="refl_square">a square [m^2] multiplied with reflection coefficient</param>
/// <param name="obs">observer's position in GCS</param>
/// <param name="sun">solar position in GCS</param>
/// <param name="obj">satellite's position in GCS</param>
/// <returns>visual magnitude value</returns>
template<geometry_type type = geometry_type::diffuse_sphere>
double apparent_magnitude(double refl_square, const ball::XYZ& obs, const ball::XYZ& sun, const ball::XYZ& obj) {
	static_assert(type >= geometry_type::diffuse_plate && type <= geometry_type::diffuse_sphere, "invalid geometry");
	double phase_func = std::numeric_limits<double>::infinity();
	if constexpr (type == geometry_type::diffuse_plate) {
		phase_func = plate_phase_function(obs, sun);
	}
	else if constexpr (type == geometry_type::diffuse_sphere) {
		phase_func = sphere_phase_function(obs, sun, obj);
	}

	double apparent_magnitude_impl(
		double refl_func, 
		const ball::XYZ& obs, 
		const ball::XYZ& sun, 
		const ball::XYZ & obj
	);

	return apparent_magnitude_impl(refl_square * phase_func, obs, sun, obj);
}