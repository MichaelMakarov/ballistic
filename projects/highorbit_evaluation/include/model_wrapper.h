#pragma once
#include <geometry_models.h>
#include <structure_type.h>


#include <forecast.h>
#include <future>

#include <optimize.h>


template<typename M, size_t N, typename ... Args>
ball::forecast<N> make_forecast(
	const ball::motion_params<N>& mp,
	const ball::JD& tk,
	const Args& ...args
)
{
	// A number of potential harmonics
	constexpr size_t harmonics_count{ 16 };

	M model(harmonics_count, args...);
	ball::forecast<N> forecast;
	forecast.run(
		mp, tk,
		[&model](const math::vec<N>& rv, const ball::JD& jd) {
			return model.acceleration(rv, jd);
		}
	);
	return forecast;
}

math::vec3 angles_from_quaternion(
	const math::quaternion& q
);

math::quaternion quaternion_from_angles(
	const math::vec3& angles
);

ball::XYZ normal(ball::JD jd, ball::XYZ obs, ball::XYZ obj);

using orbmeasure_iter_t = std::vector<orbit_observation>::const_iterator;

/**
 * @brief Class wrappes the computation of the linear motion parameters
 */
class linear_motion_wrapper {
protected:
	ball::motion_params<6> _mp;
	ball::JD _tk;
	orbmeasure_iter_t _begin, _end;

public:
	linear_motion_wrapper(
		const ball::motion_params<6>& mp,
		orbmeasure_iter_t beg, orbmeasure_iter_t end
	);
	
	const ball::JD& finaljd() const { return _tk; }
	const ball::motion_params<6> motion_parameters() const { return _mp; }

};



class basic_linear_wrapper : public linear_motion_wrapper, public math::data_interface<6, 3> {
public:
	using linear_motion_wrapper::linear_motion_wrapper;

	size_t points_count() const override {
		return static_cast<size_t>(std::distance(_begin, _end));
	}

	void update(
		math::array_view<6> param_corrections
	) override;

	void compute(
		std::vector<math::array_view<3>>& residuals,
		std::vector<std::array<math::array_view<3>, 6>>& derivatives
	) const override;
};

class extended_linear_wrapper : public linear_motion_wrapper, public math::data_interface<7, 3> {
	rotation_params _rp;
	round_plane_info _plane;
public:
	extended_linear_wrapper(
		const ball::motion_params<6>& mp,
		const rotation_params& rp,
		const round_plane_info& plane,
		orbmeasure_iter_t beg, orbmeasure_iter_t end
	);

	size_t points_count() const override {
		return static_cast<size_t>(std::distance(_begin, _end));
	}

	void update(
		math::array_view<7> param_corrections
	) override;

	void compute(
		std::vector<math::array_view<3>>& residuals,
		std::vector<std::array<math::array_view<3>, 7>>& derivatives
	) const override;

	const auto& rotation_parameters() const { return _rp; }
	const auto& object_parameters() const { return _plane; }
};

using rotational_measurement_iter = std::vector<rotational_observation>::const_iterator;

/**
 * @brief Class wrappes the computation of the rotational parameters of motion.
 */
class angular_motion_wrapper : public math::data_interface<4, 1> {
	ball::motion_params<6> _mp;
	rotation_params _rp;
	ball::JD _tk;
	rotational_measurement_iter _begin, _end;
	round_plane_info _plane;
public:
	angular_motion_wrapper(
		const ball::motion_params<6>& mp,
		const rotation_params& rp,
		rotational_measurement_iter beg, rotational_measurement_iter end, 
		const round_plane_info& plane_info
	);

	size_t points_count() const override { 
		return static_cast<size_t>(std::distance(_begin, _end)); 
	}
	void compute(
		std::vector<math::array_view<1>>& residuals,
		std::vector<std::array<math::array_view<1>, 4>>& derivatives
	) const override;

	void update(math::array_view<4> corrections) override;

	const ball::motion_params<6>& linear_params() const { return _mp; }
	const rotation_params& angular_params() const { return _rp; }
	const ball::JD& finaljd() const { return _tk; }
	const round_plane_info& plane_info() const { return _plane; }
};

/**
 * @brief Computes parameters of rotational motion.
 * @param mp parameters of motion to compute orbit from
 * @param beg an iterator of the first observation
 * @param end an iterator of the end
 * @param axis_inclination an inclination of the axis of rotation
 * @param axis_ascension a right ascension of the axis of rotation
 * @param angular_velocity an angular velocity of rotation around the axis
 */
void compute_rotation(
	const ball::motion_params<6> mp,
	rotational_measurement_iter beg, rotational_measurement_iter end,
	double& axis_inclination,
	double& axis_ascension,
	double& angular_velocity
);


