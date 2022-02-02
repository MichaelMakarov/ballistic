#pragma once
#include <geopotential.h>
#include <transform.h>

namespace ball {

	class MGPT {
		geopotential _gpt;
		double _w, _fl, _rad;
	public:
		double minheight{}, maxheight{ 1e8 };
	public:
		MGPT(const egm_interface& egm, size_t harmonics);
		~MGPT() = default;

		math::vec<6> acceleration(const math::vec<6>& vc, const JD& jd);
	};

	/// vector (rx, ry, rz, vx, vy, vz, q0, q1, q2, q3, wx, wy, wz)
	using vec13 = math::vec<13>;

	/// <summary>
	/// Surface parameters
	/// </summary>
	struct surface_params {
		math::vec3 center;	// a position of surface center
		math::vec3 norm;	// a normal vector
		double eps{};		// an optical parameter
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

	/// A model of a linear and rotational motion.
	/// A model utilizes gravitational model of the Earth and solar radiation pressure model.
	class MSLP {
		geopotential _geopotential;
		double _w, _fl, _rad;
		math::vec3 _av, _bv;
	public:
		const object_model& object;
		double minheight{ 1e5 }, maxheight{ 1e8 };
	public:
		MSLP(const earth_model& gravity, size_t harmonics, const object_model& model);
		~MSLP() = default;
		/// updating model paramters
		void update();
		/// <summary>
		/// computing the acceleration
		/// </summary>
		/// <param name="vec">is a vector that describes the position and orientation</param>
		/// <param name="jd">is a julian date characterizing the moment of time</param>
		/// <returns>a vector of acceleration of the same size</returns>
		vec13 acceleration(const vec13& vec, const JD& jd);
	private:
		//void srp_effect(const math::vec3& pos, const math::quaternion& qrot, const JD& jd, math::vec3& force, math::vec3& torque);
	};

	/// <summary>
	/// Computes a stellar magnitude of satellite.
	/// </summary>
	/// <param name="square">square of surface</param>
	/// <param name="distsqr">square of distance between observer and satellite</param>
	/// <param name="reflc">coefficient of reflection of surface</param>
	/// <param name="phase_angle">phase angle between observer - satellite - sun</param>
	/// <returns></returns>
	double stellar_magnitude(double square, double distsqr, double reflc, double phase_angle);
}