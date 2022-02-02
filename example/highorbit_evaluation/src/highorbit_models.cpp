#include "highorbit_models.h"
#include "mathlib/linalg.h"

namespace ball {

	double stellar_magnitude(double square, double distsqr, double reflc, double phase_angle) {
		// eclipse condition 
		if (phase_angle > math::PI) return 0;
		double phase_func = 2 / (3 * math::sqr(math::PI)) * ((math::PI - phase_angle) * std::cos(phase_angle) + std::sin(phase_angle));
		return -26.58 - 2.5 * std::log10(square * reflc * phase_func / distsqr);
	}

	MGPT::MGPT(const earth_model& egm, size_t harmonics) : _gpt{ geopotential(egm, harmonics) } {
		_rad = egm.rad();
		_fl = egm.flat();
		_w = egm.ang_vel();
	}

	math::vec<6> MGPT::acceleration(const math::vec<6>& vc, const times::juliandate& jd) {
		double ew2 = math::sqr(_w);
		double h = earth_model::height_above_ellipsoid(vc[0], vc[1], vc[2], _rad, _fl);
		if (h < minheight || h > maxheight) throw std::runtime_error("height out of bounds");
		auto potv = _gpt.acceleration(vc[0], vc[1], vc[2]);
		return math::vec<6>{
			vc[3], vc[4], vc[5],
			potv[0] + ew2 * vc[0] + 2 * _w * vc[4],
			potv[1] + ew2 * vc[1] - 2 * _w * vc[3],
			potv[2]
		};
	}

	// verifying if point in eclipse of the Earth relative to the sun
	double shadow_function(const math::vec3 solar_pos, const math::vec3& point_pos, double erad);

	MSLP::MSLP(const earth_model& gravity, size_t harmonics, const object_model& model) :
		_potential{ geopotential(gravity, harmonics) },
		object{ model }
	{
		_w = gravity.ang_vel();
		_fl = gravity.flat();
		_rad = gravity.rad();
		update();
	}

	void MSLP::update() {
		// main torques of inertia  J = (Jx, Jy, Jz)
		_bv[0] = 1 / object.inertia[0];	// 1 / Jx
		_bv[1] = 1 / object.inertia[1];	// 1 / Jy
		_bv[2] = 1 / object.inertia[2];	// 1 / Jz
		_av[0] = (object.inertia[1] - object.inertia[2]) * _bv[0];	// (Jy - Jz) / Jx
		_av[1] = (object.inertia[2] - object.inertia[0]) * _bv[1];	// (Jz - Jx) / Jy
		_av[2] = (object.inertia[0] - object.inertia[1]) * _bv[2];	// (Jx - Jy) / Jz
	}

	vec13 MSLP::acceleration(const vec13& vec, const times::juliandate& jd) {
		double ew2 = math::sqr(_w);
		math::vec3 pos{ vec[0], vec[1], vec[2] };	// radius vector
		math::vec3 vel{ vec[3], vec[4], vec[5] };	// linear velocity
		math::quaternion qrot{ vec[6], math::slice<7, 9>(vec) };	// quaternion
		auto qvel = math::quaternion{ 0, math::slice<10, 12>(vec) } * qrot * 0.5;	// derivative of quaternion

		// normalization of quaternion and orthogonalization of its derivative
		qrot /= qrot.mod();
		qvel -= qrot * math::dot(qrot, qvel);

		// verfying if the spacecraft is in height bounds
		double h = earth_model::height_above_ellipsoid(pos[0], pos[1], pos[2], _rad, _fl);
		if (minheight > h || h > maxheight) throw std::runtime_error("height out of bounds");
		// acceleration caused by geopotential
		auto potv = _potential.acceleration(pos[0], pos[1], pos[2]);

		math::vec3 torques, forces;
		srp_effect(pos, qrot, jd, forces, torques);
		forces = math::rotate_vector(forces, qrot);

		/**
		* angular velocity from derivative of quaternion and inverse quaternion
		* w = 2 * dq/dt * q^-1
		* wx = 2(-q1 * dq0 + q0 * dq1 + q3 * dq2 - q2 * dq3)
		* wy = 2(-q2 * dq0 - q3 * dq1 + q0 * dq2 + q1 * dq3)
		* wz = 2(-q3 * dq0 + q2 * dq1 - q1 * dq2 + q0 * dq3)
		*/
		//auto&& angvel = (qvel * conj(qrot) * 2.0).v;	// inverse quaternion equals conjugated one because qrot is versor
		//double angvel2 = math::sqr(angvel) * -0.25;
		/**
		* right part of the equation of the rotational motion:
		* J * dw/dt + ( (Jz - Jy) * wy * wz, (Jx - Jz) * wx * wz, (Jy - Jx) * wx * wy ) = M
		* other words: dw/dt = J^-1 * ( M - w x (J * w) ) 
		*/
		return vec13{
			vel[0], vel[1], vel[2],
			forces[0] + potv[0] + ew2 * pos[0] + 2 * _w * vel[1],
			forces[1] + potv[1] + ew2 * pos[1] - 2 * _w * vel[0],
			forces[2] + potv[2],
			qvel.s, qvel.v[0], qvel.v[1], qvel.v[2],
			vec[10 + 1] * vec[10 + 2] * _av[0] + _bv[0] * torques[0],
			vec[10 + 2] * vec[10 + 0] * _av[1] + _bv[1] * torques[1],
			vec[10 + 0] * vec[10 + 1] * _av[2] + _bv[2] * torques[2]
			//angvel2 * qrot.s	+ 0.5 * (-qrot.v[0] * fn[0] - qrot.v[1] * fn[2] - qrot.v[2] * fn[2]),
			//angvel2 * qrot.v[0] + 0.5 * ( qrot.s	* fn[0] - qrot.v[2] * fn[2] + qrot.v[1] * fn[2]),
			//angvel2 * qrot.v[1] + 0.5 * ( qrot.v[2] * fn[0] + qrot.s	* fn[2] - qrot.v[0] * fn[2]),
			//angvel2 * qrot.v[2] + 0.5 * (-qrot.v[1] * fn[0] + qrot.v[0] * fn[2] + qrot.s	* fn[2])
		};
	}

	void MSLP::srp_effect(const math::vec3& pos, const math::quaternion& qrot, const times::juliandate& jd, math::vec3& force, math::vec3& torque) {
		solar_model sol(jd);
		auto solpos = ACS_to_GCS(sol.orth_position(), earth_model::sidereal_time_mean(jd));
		// eclipse function divided by mass of satellite (minus due to the force direction is opposite to the sunsat vector) 
		double eclipse = -shadow_function(solpos, pos, _rad) / object.mass;
		// if eclipse condition
		if (eclipse == 0.0) return;
		// a unit vector from space vehicle to sun
		// rotating the sunsat vector to the coordinate system connected with the object
		auto sunsat_vec = math::rotate_vector(normalize(solpos - pos), math::conj(qrot));

		for (const auto& face : object.surface) {
			double cos_angle = face.norm * sunsat_vec;
			if (cos_angle > 0) {
				// coefficient of multiplication
				auto face_force = (eclipse * solar_model::solar_pressure() * cos_angle * face.square) * 
					((1 - face.eps) * sunsat_vec + (2 * face.eps * cos_angle) * face.norm);
				// computing the force occured due to absobing and reflecting light beams
				force += face_force;
				torque += math::cross(face.center, face_force);
			}
		}

		//force = math::rotate_vector(force, qrot);
		//torque = math::rotate_vector(torque, qrot);
	}

	bool point_inside_cone(const math::vec3& point, double disp, double erad) {
		double sin_angle = erad / disp;
		double cos_angle2 = 1 - math::sqr(sin_angle);
		if (point[0] > erad * sin_angle) return false;
		return ((math::sqr(point[1]) + math::sqr(point[2])) / cos_angle2 - math::sqr((point[0] - disp) / sin_angle)) < 0.0;
	}
	
	double shadow_function(const math::vec3 solar_pos, const math::vec3& point_pos, double erad) {
		// transform matrix to coordinate system where z coordinate is directed to the sun
		auto mx_transform = math::transform_matrix_from_vector(solar_pos);
		// a position of the satellite in new coordinate system
		auto transf_pos = mx_transform * point_pos;
		double dist = solar_pos.length();
		// solar radius to the Earth's radius ratio
		double coef = solar_model::rad() / erad;
		// computing point position relative to umbra and penumbra cones
		bool inside_umbra = point_inside_cone(transf_pos, -dist / (coef - 1), erad);
		bool inside_penumbra = point_inside_cone(transf_pos, dist / (coef + 1), erad);
		// umbra condition
		if (inside_umbra) return 0.0;
		// lighted condition
		if (!inside_penumbra) return 1.0;
		// penumbra condition
		return 0.5;
	}
}