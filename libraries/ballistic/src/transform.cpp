#include <transform.h>
#include <linalg.h>

namespace ball {
	math::vec3 ort_to_sph(const math::vec3& vec) {
		return math::vec3{
			length(vec),
			std::atan2(vec[2], std::sqrt(vec[0] * vec[0] + vec[1] * vec[1])),
			std::atan2(vec[1], vec[0])
		};
	}

	math::vec3 sph_to_ort(const math::vec3& vec) {
		double coslat = std::cos(vec[1]);
		return math::vec3{
			vec[0] * std::cos(vec[2]) * coslat,
			vec[0] * std::sin(vec[2]) * coslat,
			vec[0] * std::sin(vec[1])
		};
	}

	math::vec3 ACS_to_GCS(const math::vec3& vec, double sidereal_time) {
		double sint = std::sin(sidereal_time), cost = std::cos(sidereal_time);
		return math::vec3{
			vec[0] * cost + vec[1] * sint,
			vec[1] * cost - vec[0] * sint,
			vec[2]
		};
	}

	math::vec3 GCS_to_ACS(const math::vec3& vec, double sidereal_time) {
		double sint = std::sin(sidereal_time), cost = std::cos(sidereal_time);
		return math::vec3{
			vec[0] * cost - vec[1] * sint,
			vec[1] * cost + vec[0] * sint,
			vec[2]
		};
	}

	math::vec3 ECS_to_ACS(const math::vec3& vec, double ecl_incl) {
		double sine = std::sin(ecl_incl), cose = std::cos(ecl_incl);
		return math::vec3{
			vec[0],
			vec[1] * cose - vec[2] * sine,
			vec[1] * sine + vec[2] * cose
		};
	}

	math::vec3 ACS_to_ECS(const math::vec3& vec, double ecl_incl) {
		double sine = std::sin(ecl_incl), cose = std::cos(ecl_incl);
		return math::vec3{
			vec[0],
			vec[2] * sine + vec[1] * cose,
			vec[2] * cose - vec[1] * sine
		};
	}

	math::vec<6> ACS_to_GCS(const math::vec<6>& vec, double sidereal_time, double angvel) {
		double sint = std::sin(sidereal_time), cost = std::cos(sidereal_time);
		// X = G * Xa
		// dX = dG * Xa + G * dXa
		// G - matrix of conversion
		math::vec<6> result{
			vec[0] * cost + vec[1] * sint,
			vec[1] * cost - vec[0] * sint,
			vec[2],
			vec[3] * cost + vec[4] * sint,
			vec[4] * cost - vec[3] * sint,
			vec[5]
		};
		result[3] += angvel * result[1];
		result[4] -= angvel * result[0];
		return result;
	}

	math::vec<6> GCS_to_ACS(const math::vec<6>& vec, double sidereal_time, double angvel) {
		double sint = std::sin(sidereal_time), cost = std::cos(sidereal_time);
		math::vec<6> result{
			vec[0] * cost - vec[1] * sint,
			vec[1] * cost + vec[0] * sint,
			vec[2],
			vec[3] * cost - vec[4] * sint,
			vec[4] * cost + vec[3] * sint,
			vec[5]
		};
		result[3] -= angvel * result[1];
		result[4] += angvel * result[0];
		return result;
	}

	orbital_params orbparams_from_motionvec(const math::vec3& pos, const math::vec3& vel, double mu) {
		using namespace math;
		orbital_params osc;
		auto w = cross(pos, vel);
		double p{ w * w / mu };
		normalize(w);
		double r{ length(pos) };
		osc.inclination = std::atan(std::sqrt(w[0] * w[0] + w[1] * w[1]) / w[2]);
		osc.ascendnode = std::atan2(w[0], -w[1]);
		osc.semiaxis = 1 / (2 / length(pos) - vel * vel / mu);
		osc.eccentricity = std::sqrt(1 - p / osc.semiaxis);
		osc.ecanomaly = std::atan((pos * vel) / std::sqrt(osc.semiaxis * mu) / (1 - r / osc.semiaxis));
		osc.meananomaly = osc.ecanomaly - osc.eccentricity * std::sin(osc.ecanomaly);
		osc.trueanomaly = std::atan2(
			std::sqrt(1 - osc.eccentricity * osc.eccentricity) * std::sin(osc.ecanomaly),
			std::cos(osc.ecanomaly) - osc.eccentricity);
		osc.latitudearg = std::atan2(pos[2], pos[1] * w[0] - pos[0] * w[1]);
		osc.periapsis = osc.trueanomaly - osc.latitudearg;
		return osc;
	}

	enum class transform_type {
		FORWARD, BACKWARD
	};

	template<transform_type type>
	math::mat3x3 GCS_to_OCS_impl(math::vec3& r, math::vec3& v)
	{
		math::mat3x3 mx;
		math::normalize(r);
		math::normalize(v);
		const auto& b = math::cross(r, v);
		const auto& l = math::cross(r, b);
		if constexpr (type == transform_type::FORWARD) {
			for (size_t i{}; i < 3; ++i) {
				mx(0, i) = r[i];
				mx(1, i) = b[i];
				mx(2, i) = l[i];
			}
		}
		else {
			for (size_t i{}; i < 3; ++i) {
				mx(i, 0) = r[i];
				mx(i, 1) = b[i];
				mx(i, 2) = l[i];
			}
		}
		return mx;
	}

	math::mat3x3 GCS_to_OCS(const math::vec<6>& rv)
	{
		math::mat3x3 mx;
		auto r = math::slice<0, 2>(rv);
		math::normalize(r);
		for (size_t i{}; i < 3; ++i) mx(0, i) = r[i];
		auto l = -math::slice<3, 5>(rv);
		math::normalize(l);
		l = math::cross(r, l);
		for (size_t i{}; i < 3; ++i) mx(1, i) = l[i];
		l = math::cross(r, l);
		for (size_t i{}; i < 3; ++i) mx(2, i) = l[i];
		return mx;
	}

	math::mat3x3 GCS_to_OCS(math::vec3 r, math::vec3 v)
	{
		return GCS_to_OCS_impl<transform_type::FORWARD>(r, v);
	}

	math::mat3x3 OCS_to_GCS(math::vec3 r, math::vec3 v)
	{
		return GCS_to_OCS_impl<transform_type::BACKWARD>(r, v);
	}
}