#include <highorbit_models.h>
#include <solar_system.h>

#include <mathconstants.h>
#include <transform.h>

//#include <conversion.h>
#include <linalg.h>

#include <format>


using namespace ball;

/**
 * The structure describes the solar radiation pressure effect 
 */
struct srp_effect {
	math::vec3 linear;		// linear acceleration
	math::vec3 angular;		// angular acceleration
	math::vec4 dquat;		// a derivative of the quaternion of rotation
};

/**
 * @brief Computes solar radiation pressure effect.
 * @param pos coordinates of position
 * @param rt a quaternion that describes rotation
 * @param vr a derivative of the quaternion
 * @param sun solar position
 * @param earth_radius a radius of the Earth [m]
 * @param model a pointer to the model of the object
 * @param angcoef coefficients describes the parameters of inertia
 * @param trqcoef coefficients before the exterior torques
 * @return an effect of the solar radiation pressure
 */
srp_effect solar_pressure(
	const XYZ& pos,
	const math::vec4& rt,
	const XYZ& vr,
	const ball::XYZ& sun,
	double earth_radius,
	const object_model* model,
	const ball::XYZ& angcoef,
	const ball::XYZ& trqcoef
);


double plate_phase_function(const XYZ& obs, const XYZ& sun)
{
	double sin_of_sun_lat{ sun[2] / math::length(sun) };
	double sin_of_obs_lat{ obs[2] / math::length(obs) };
	double mult{ sin_of_obs_lat * sin_of_sun_lat };
	return mult > 0 ? mult / math::PI : 0;
}

double sphere_phase_function(
	const XYZ& obs, 
	const XYZ& sun, 
	const ball::XYZ& sat
)
{
	auto sunsat = sat - sun;
	auto obssat = sat - obs;
	math::normalize(sunsat);
	math::normalize(obssat);
	double cos_phase = sunsat * obssat;
	double sin_phase = std::sqrt(1 - math::sqr(cos_phase));
	double phase_angle = std::acos(cos_phase);
	return 2 / (3 * math::sqr(math::PI)) * ((math::PI - phase_angle) * cos_phase + sin_phase);
}

double apparent_magnitude_impl(
	double refl_func, 
	const ball::XYZ& obs, 
	const ball::XYZ& sun, 
	const ball::XYZ& obj
)
{
	return -26.58 - 2.5 * std::log10(refl_func / math::sqr(obs - obj));
}

basic_motion_model::basic_motion_model(size_t harmonics)
{
	_rad = EGM96::rad;
	_fl = EGM96::flat;
	_w = EGM96::angv;
	_gpt = geopotential(ball::EGM96{}, harmonics);
}

RV linear_motion_model::acceleration(const RV& vc, const JD& jd)
{
	double ew2 = math::sqr(_w);
	double h = height_above_ellipsoid(vc[0], vc[1], vc[2], _rad, _fl);
	if (h < minheight || h > maxheight) {
		throw std::runtime_error(
			std::format("height = {} is out of bounds [{}, {}]", h, minheight, maxheight)
		);
	}

	auto gpt = _gpt.acceleration(vc[0], vc[1], vc[2]);

	return {
		vc[3], vc[4], vc[5],
		gpt[0] + ew2 * vc[0] + 2 * _w * vc[4],
		gpt[1] + ew2 * vc[1] - 2 * _w * vc[3],
		gpt[2],

	};
}

angular_motion_model::angular_motion_model(
	size_t harmonics,
	const object_model* const obj
) : basic_motion_model(harmonics)
{
	_obj = obj;

	// main torques of inertia  J = (Jx, Jy, Jz)
	_bv[0] = 1 / _obj->inertia[0];	// 1 / Jx
	_bv[1] = 1 / _obj->inertia[1];	// 1 / Jy
	_bv[2] = 1 / _obj->inertia[2];	// 1 / Jz
	_av[0] = (_obj->inertia[1] - _obj->inertia[2]) * _bv[0];	// (Jy - Jz) / Jx
	_av[1] = (_obj->inertia[2] - _obj->inertia[0]) * _bv[1];	// (Jz - Jx) / Jy
	_av[2] = (_obj->inertia[0] - _obj->inertia[1]) * _bv[2];	// (Jx - Jy) / Jz
}

RVE angular_motion_model::acceleration(
	const RVE& vc, 
	const JD& jd
)
{
	double ew2 = math::sqr(_w);
	double h = height_above_ellipsoid(vc[0], vc[1], vc[2], _rad, _fl);
	if (h < minheight || h > maxheight) {
		throw std::runtime_error(
			std::format("height = {} is out of bounds [{}, {}]", h, minheight, maxheight)
		);
	}

	auto gpt = _gpt.acceleration(vc[0], vc[1], vc[2]);

	auto sun = sph_to_ort(solar_model::position(jd));
	auto sidt = sidereal_time_mean(jd);
	auto [
		linear, 
		angular, 
		dq
	] = solar_pressure(
		GCS_to_ACS(math::slice<0, 2>(vc), sidt),
		math::slice<6, 9>(vc),
		math::slice<10, 12>(vc),
		sun, 
		_rad, 
		_obj, 
		_av, _bv
	);

	linear = ACS_to_GCS(linear, sidt);

	return {
		vc[3], vc[4], vc[5],
		linear[0] + gpt[0] + ew2 * vc[0] + 2 * _w * vc[4],
		linear[1] + gpt[1] + ew2 * vc[1] - 2 * _w * vc[3],
		linear[2] + gpt[2],
		dq[0], dq[1], dq[2], dq[3],
		angular[0], angular[1], angular[2]
	};
}

bool point_inside_cone(
	const math::vec3& point,
	double cone_apex, 
	double erad
) 
{
	// a square of distance from x axis to point
	double hsqr{ math::sqr(point[1]) + math::sqr(point[2]) };
	// a square of distance from cone apex to point
	double dsqr{ hsqr + math::sqr(point[0] - cone_apex) };
	// compare the squares of sinus of angle
	// if sin_angle corresponding to point is greater than 
	// sin_angle of the cone
	return hsqr / dsqr < math::sqr(erad / cone_apex);
}

double eclipse_condition(
	const math::vec3 solar_pos, 
	const math::vec3& point_pos,
	double erad
) {
	// transform matrix to coordinate system where z coordinate is directed to the sun
	auto mx_transform = math::make_transform(solar_pos);
	// a position of the satellite in new coordinate system
	auto transf_pos = mx_transform * point_pos;
	// a distance to the sun from the earth
	double dist = length(solar_pos);
	// solar radius to the Earth's radius ratio
	double coef = solar_model::rad() / erad;
	// umbra condition
	bool inside_umbra = point_inside_cone(transf_pos, -dist / (coef - 1), erad);
	// penumbra condition
	bool inside_penumbra = point_inside_cone(transf_pos, dist / (coef + 1), erad);

	return inside_umbra ? 0 : (inside_penumbra ? 0.5 : 1);
}

srp_effect solar_pressure(
	const XYZ& pos,
	const math::vec4& rt,
	const XYZ& vr,
	const ball::XYZ& sun,
	double erad,
	const object_model* model,
	const ball::XYZ& angcoef,
	const ball::XYZ& trqcoef
)
{
	srp_effect output;

	math::quaternion qr{ rt[0], { rt[1], rt[2], rt[3] } };	// quaternion
	qr /= mod(qr);											// normalization
	math::quaternion qv{ 0, vr };							// derivative of quaternion
	qv *= qr * 0.5;
	qv -= qr * math::dot(qr, qv);							// orthogonalization

	std::memcpy(output.dquat.data(), &qv, sizeof(qv));

	// eclipse function divided by mass of satellite (minus due to the force direction is opposite to the sunsat vector) 
	double eclipse = -eclipse_condition(sun, pos, erad) / model->mass;
	// if eclipse condition
	if (eclipse < 0) {
		// a unit vector from space vehicle to sun in orbital coordinate system
		auto sunsat_vec = sun - pos;
		// transform vector to object referenced coordinate system
		sunsat_vec = math::rotate_vector(sunsat_vec, math::conj(qr));
		math::normalize(sunsat_vec);

		// pass throw all faces to compute all forces and torques
		for (const auto& face : model->surface) {
			// angle between sunsat vector and face normal
			double cos_angle = face.norm * sunsat_vec;
			if (cos_angle > 0) {
				// a force caused by solar radiation
				auto face_force = (1 - face.refl) * sunsat_vec + (2 * face.refl * cos_angle) * face.norm;
				face_force *= eclipse * solar_model::solar_pressure() * cos_angle * face.square;
				// computing the force occured due to absobing and reflecting light beams
				output.linear += face_force;
				output.angular += math::cross(face.center, face_force);
			}
		}
		/**
		* compute the torques in body referenced coordinate system
		* right part of the equation of the rotational motion:
		* J * dw/dt + ( (Jz - Jy) * wy * wz, (Jx - Jz) * wx * wz, (Jy - Jx) * wx * wy ) = M
		* other words: dw/dt = J^-1 * ( M - w x (J * w) )
		*/
		output.angular[0] = vr[1] * vr[2] * angcoef[0] + trqcoef[0] * output.angular[0];
		output.angular[1] = vr[2] * vr[0] * angcoef[1] + trqcoef[1] * output.angular[1];
		output.angular[2] = vr[0] * vr[1] * angcoef[2] + trqcoef[2] * output.angular[2];
		// conversion to orbital cooedinate system
		output.linear = math::rotate_vector(output.linear, qr);
		// conversion to greenwich coordinate system
		output.linear = output.linear;
	}

	return output;
}

ball::XYZ solar_pressure(
	const ball::XYZ& pos, const math::quaternion& qr,
	const ball::XYZ& sun, double erad,
	const object_model* obj
)
{
	ball::XYZ accel;
	// eclipse function divided by mass of satellite (minus due to the force direction is opposite to the sunsat vector) 
	double eclipse = -eclipse_condition(sun, pos, erad);
	// if eclipse condition
	if (eclipse < 0) {
		// a unit vector from space vehicle to sun in orbital coordinate system
		auto sunsat_vec = sun - pos;
		eclipse *= solar_model::solar_pressure() * math::sqr(solar_model::AU()) / (math::sqr(sunsat_vec) * obj->mass);
		// transform vector to object referenced coordinate system
		sunsat_vec = math::rotate_vector(sunsat_vec, math::conj(qr));
		math::normalize(sunsat_vec);
		// pass throw all faces to compute all forces and torques
		for (const auto& face : obj->surface) {
			// angle between sunsat vector and face normal
			double cos_angle = face.norm * sunsat_vec;
			if (cos_angle > 0) {
				// a force caused by solar radiation
				auto face_force = (1 - face.refl) * sunsat_vec + (2 * face.refl * cos_angle) * face.norm;
				face_force *= cos_angle * face.square;
				// computing the force occured due to absobing and reflecting light beams
				accel += face_force;
			}
		}
		accel *= eclipse;
	}
	return math::rotate_vector(accel, qr);
}

extended_motion_model::extended_motion_model(
	size_t harmonics,
	const rotation_params& rp,
	const object_model* obj
) : linear_motion_model(harmonics)
{
	_obj = obj;
	_rp = rp;
}

RV extended_motion_model::acceleration(const RV& vc, const ball::JD& jd)
{
	auto ac = linear_motion_model::acceleration(vc, jd);
	
	auto sun = sph_to_ort(solar_model::position(jd));
	double sidt = sidereal_time_mean(jd);
	auto pos = GCS_to_ACS(math::slice<0, 2>(vc), sidt);
	
	auto sa = solar_pressure(pos, _rp.rotation(jd), sun, _rad, _obj);
	sa = ACS_to_GCS(sa, sidt);

	ac[3] += sa[0];
	ac[4] += sa[1];
	ac[5] += sa[2];

	return ac;
}

math::quaternion rotation_params::rotation(const ball::JD& jd) const
{
	// the result rotation q = q2 * q1 when rotating by q1 and then by q2
	return make_quaternion(axis, vel * (jd - this->jd)) * quat;
}
