#include <model_wrapper.h>

#include <quaternion.h>

#include <future>

#include <solar_system.h>
#include <transform.h>


constexpr std::array gl_vars{
	25.0,		// position
	0.25,		// velocity
	0.05,		// angle of rotation
	0.01,		// axis of rotation
	0.01,		// velocity of rotation
	0.1,		// surface dimension
	0.01		// center of mass position
};
/** @brief position variation */
constexpr double pos_var{ 25 };
/** @brief velocity variation */
constexpr double vel_var{ 0.25 };
/** @brief surface dimension variation */
constexpr double surf_var{ 0.1 };
/** @brief reflection coefficient variation */
constexpr double refl_var{ 0.01 };

math::vec3 angles_from_quaternion(
	const math::quaternion& q
)
{
	auto [
		axis,	// axis of rotation
		phi		// angle of rotation
	] = from_quaternion(q);
	if (axis[2] < 0) {
		axis = -axis;
		phi = -phi;
	}
	auto vc = ball::ort_to_sph(axis);
	vc[0] = phi;
	return vc;
}

math::quaternion quaternion_from_angles(
	const math::vec3& angles
)
{
	auto axis = ball::sph_to_ort({ 1, angles[1], angles[2] });
	return math::make_quaternion(axis, angles[0]);
}


linear_motion_wrapper::linear_motion_wrapper(
	const ball::motion_params<6>& mp,
	orbmeasure_iter_t beg, orbmeasure_iter_t end
)
{
	_mp = mp;
	_begin = beg;
	_end = end;
	_tk = std::max_element(
		beg, end, 
		[](const auto& left, const auto& right) { return left.jd < right.jd; }
	)->jd;
}

extended_linear_wrapper::extended_linear_wrapper(
	const ball::motion_params<6>& mp,
	const rotation_params& rp,
	const round_plane_info& plane,
	orbmeasure_iter_t beg, orbmeasure_iter_t end
) : linear_motion_wrapper(mp, beg, end)
{
	_rp = rp;
	_plane = plane;
}

void basic_linear_wrapper::compute(
	std::vector<math::array_view<3>>& residuals,
	std::vector<std::array<math::array_view<3>, 6>>& derivatives
) const
{
	// variations of the parameters of motion
	constexpr double variations[6] {
		gl_vars[0], gl_vars[0], gl_vars[0],
		gl_vars[1], gl_vars[1], gl_vars[1]
	};
	// a function of forecast computation
	auto compute_func = &make_forecast<linear_motion_model, 6>;
	// an array of results
	std::array<std::future<ball::forecast<6>>, 6> forecasts;
	
	for (size_t i{ 0 }; i < forecasts.size(); ++i) {
		auto mp = _mp;
		mp.vec[i] += variations[0];
		forecasts[i] = std::async(std::launch::async, compute_func,	mp, _tk);
	}
	auto forecast = compute_func(_mp, _tk);

	size_t point_index{};
	std::vector<ball::motion_params<6>> mplist(residuals.size());
	for (auto it = _begin; it != _end; ++it, ++point_index) {
		mplist[point_index] = forecast.get_point(it->jd);
		for (size_t coord_index{}; coord_index < 3; ++coord_index) {
			residuals[point_index][coord_index] = it->vc[coord_index] - mplist[point_index].vec[coord_index];
		}
	}

	for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
		forecast = forecasts[param_index].get();
		for (point_index = 0; point_index < mplist.size(); ++point_index) {
			auto mp = forecast.get_point(mplist[point_index].jd);
			for (size_t coord_index{}; coord_index < 3; ++coord_index) {
				derivatives[point_index][param_index][coord_index] = 
					(mp.vec[coord_index] - mplist[point_index].vec[coord_index]) / variations[param_index];
			}
		}
	}
}

void basic_linear_wrapper::update(math::array_view<6> corrections)
{
	for (size_t i{}; i < 6; ++i) _mp.vec[i] += corrections[i];
}

void extended_linear_wrapper::compute(
	std::vector<math::array_view<3>>& residuals,
	std::vector<std::array<math::array_view<3>, 7>>& derivatives
) const
{
	// variations of the parameters of motion
	constexpr double variations[]{
		pos_var, pos_var, pos_var,
		vel_var, vel_var, vel_var,
		surf_var, refl_var
	};
	// a function of forecast computation
	auto compute_func = &make_forecast<extended_motion_model, 6, const rotation_params&, const object_model*>;
	// an array of results
	std::array<std::future<ball::forecast<6>>, 7> forecasts;

	auto obj = make_round_plane(_plane);

	for (size_t i{ 0 }; i < 6; ++i) {
		auto mp = _mp;
		mp.vec[i] += variations[i];
		forecasts[i] = std::async(std::launch::async, compute_func, mp, _tk, _rp, &obj);
	}
	
	auto plane = _plane;
	plane.radius += variations[6];
	auto obj1 = make_round_plane(plane);
	forecasts[6] = std::async(std::launch::async, compute_func, _mp, _tk, _rp, &obj1);

	//plane = _plane;
	//plane.reflection += variations[7];
	//auto obj2 = make_round_plane(plane);
	//forecasts[7] = std::async(std::launch::async, compute_func, _mp, _tk, _rp, &obj2);

	auto forecast = compute_func(_mp, _tk, _rp, &obj);

	size_t point_index{};
	std::vector<ball::motion_params<6>> mplist(residuals.size());
	for (auto it = _begin; it != _end; ++it, ++point_index) {
		mplist[point_index] = forecast.get_point(it->jd);
		for (size_t coord_index{}; coord_index < 3; ++coord_index) {
			residuals[point_index][coord_index] = it->vc[coord_index] - mplist[point_index].vec[coord_index];
		}
	}

	for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
		forecast = forecasts[param_index].get();
		for (point_index = 0; point_index < mplist.size(); ++point_index) {
			auto mp = forecast.get_point(mplist[point_index].jd);
			for (size_t coord_index{}; coord_index < 3; ++coord_index) {
				derivatives[point_index][param_index][coord_index] =
					(mp.vec[coord_index] - mplist[point_index].vec[coord_index]) / variations[param_index];
			}
		}
	}
}

double minmax_value(double val, double min, double max)
{
	return std::max(min, std::min(max, val));
}

void extended_linear_wrapper::update(math::array_view<7> corrections)
{
	for (size_t i{}; i < 6; ++i) _mp.vec[i] += corrections[i];
	_plane.radius += corrections[6];
	//_plane.reflection += corrections[7];
	//_plane.reflection = minmax_value(_plane.reflection, 0, 1);
}

angular_motion_wrapper::angular_motion_wrapper(
	const ball::motion_params<6>& mp,
	const rotation_params& rp,
	rotational_measurement_iter beg, rotational_measurement_iter end,
	const round_plane_info& plane_info
)
{
	_mp = mp;
	_rp = rp;
	_begin = beg;
	_end = end;
	_plane = plane_info;
	_tk = std::max_element(
		_begin, _end, 
		[](const auto& left, const auto& right) { return left.jd < right.jd; }
	)->jd;
}


void angular_motion_wrapper::compute(
	std::vector<math::array_view<1>>& residuals,
	std::vector<std::array<math::array_view<1>, 4>>& derivatives
) const
{
//	auto q = quaternion_from_angles({ _rp.phase_angle, _rp.axis_incl, _rp.axis_asc });
//	auto w = math::rotate_vector({ _rp.velocity, 0, 0 }, q);
//	// initial parameters of motion
//	ball::motion_params<13> imp;
//	imp.loop = _mp.loop;
//	imp.jd = _mp.jd;
//	std::copy(std::begin(_mp.vec), std::end(_mp.vec), std::begin(imp.vec));
//	imp.vec[6] = q.s;
//	imp.vec[7] = q.v[0];
//	imp.vec[8] = q.v[1];
//	imp.vec[9] = q.v[2];
//	imp.vec[10] = w[0];
//	imp.vec[11] = w[1];
//	imp.vec[12] = w[2];
//	
//	constexpr double angle_var{ 0.05 };		// angular variation
//	constexpr double angvel_var{ 0.01 };	// angular velocity variation
//	constexpr double surfdim_var{ 0.1 };	// surface dimension variation
//
//	auto compute_func = &make_forecast<angular_motion_model, 13, const object_model*>;
//
//	// an array of forecasts
//	std::array<std::future<ball::forecast<13>>, 4> forecasts;
//	// a model of object
//	auto obj = make_round_plane(_plane);
//	
//	  //----------------------//
//	 //		linear motion	 //
//	//----------------------//
//	// deprecated
//
//	  //---------------------//
//	 //    angular motion   //
//	//---------------------//
//
//	size_t index{};
//	ball::motion_params<13> mp;
//
//	// angular velocity
//	mp = imp;
//	w = math::rotate_vector({ _rp.velocity + angvel_var, 0, 0 }, math::conj(q));
//	mp.vec[10] = w[0];
//	mp.vec[11] = w[1];
//	mp.vec[12] = w[2];
//	forecasts[index++] = std::async(std::launch::async, compute_func, mp, _tk, &obj);
//
//	math::vec3 angles{ _rp.phase_angle, _rp.axis_incl, _rp.axis_asc };
//	// inclination variation
//	mp = imp;
//	q = quaternion_from_angles(angles + math::vec3{ 0, angle_var, 0 });
//	mp.vec[6] = q.s;
//	mp.vec[7] = q.v[0];
//	mp.vec[8] = q.v[1];
//	mp.vec[9] = q.v[2];
//	forecasts[index++] = std::async(std::launch::async, compute_func, mp, _tk, &obj);
//
//	// ascension variation
//	mp = imp;
//	q = quaternion_from_angles(angles + math::vec3{ 0, 0, angle_var });
//	mp.vec[6] = q.s;
//	mp.vec[7] = q.v[0];
//	mp.vec[8] = q.v[1];
//	mp.vec[9] = q.v[2];
//	forecasts[index++] = std::async(std::launch::async, compute_func, mp, _tk, &obj);
//
//	// phase variation
//	mp = imp;
//	q = quaternion_from_angles(angles + math::vec3{ angle_var, 0, 0 });
//	mp.vec[6] = q.s;
//	mp.vec[7] = q.v[0];
//	mp.vec[8] = q.v[1];
//	mp.vec[9] = q.v[2];
//	forecasts[index++] = std::async(std::launch::async, compute_func, mp, _tk, &obj);
//	
//	  //-----------------------------//
//	 //		MODEL PARAMETERS		//
//	//-----------------------------//
//
//	/*auto plane_info = _plane;
//	plane_info.radius += variations[index];
//	integrators[index] = std::make_unique<angular_motion_integrator>(_mp, _tk, make_round_plane(plane_info));
//	forecasts[index] = std::async(
//		std::launch::async,
//		compute_func,
//		integrators[index].get()
//	);*/
//
//	auto forecast = compute_func(imp, _tk, &obj);
//	/**
//	 * @brief an array of motion params computed from initial parameters of motion
//	 */
//	std::vector<math::vec3> params(residuals.size());
//
//	size_t point_index{};
//	// residuals computation
//	for (auto it = _begin; it != _end; ++it, ++point_index) {
//		auto mp = forecast.get_point(it->jd);
//		auto pos = math::slice<0, 2>(mp.vec);
//		auto refn = normal(it->jd, it->obs, pos);
//		auto cmpn = rotate_vector(_plane.normal, { mp.vec[6], math::slice<7, 9>(mp.vec) });
//		residuals[point_index][0] = 1 - refn * cmpn;
//		/*double magn = apparent_magnitude<geometry_type::diffuse_sphere>(
//			_plane.square() * _plane.reflection,
//			it->obs, 
//			ball::solar_position(it->jd),
//			pos
//		);*/
//		params[point_index] = cmpn;
//	}
//	// partial derivatives computation
//	for (size_t param_index{}; param_index < forecasts.size(); ++param_index) {
//		forecast = forecasts[param_index].get();
//		point_index = 0;
//		for (auto it = _begin; it != _end; ++point_index, ++it) {
//			auto mp = forecast.get_point(it->jd);
//			auto cmpn = rotate_vector(_plane.normal, { mp.vec[6], math::slice<7, 9>(mp.vec) });
//			const auto& refn = params[point_index];
//			derivatives[point_index][param_index][0] = 1 - refn * cmpn;
//		}
//	}
}

void angular_motion_wrapper::update(math::array_view<4> corrections)
{
	/*_rp.velocity += corrections[0];
	_rp.axis_incl += corrections[1];
	_rp.axis_asc += corrections[2];
	_rp.phase_angle += corrections[3];*/
}


ball::XYZ normal(
	ball::JD jd,
	ball::XYZ obs, 
	ball::XYZ sat
)
{
	auto sidt = ball::sidereal_time_mean(jd);
	sat = ball::GCS_to_ACS(sat, sidt);
	obs = ball::GCS_to_ACS(obs, sidt);
	auto sun = ball::sph_to_ort(ball::solar_model::position(jd));
	sun -= sat;
	obs -= sat;
	math::normalize(sun);
	math::normalize(obs);
	sat = sun + obs;
	math::normalize(sat);
	return sat;
}

math::vec3 normal(rotational_observation o, const ball::RV& v) {
	return normal(o.jd, o.obs, math::slice<0, 2>(v));
}

void compute_rotation(
	const ball::motion_params<6> mp, 
	rotational_measurement_iter beg, rotational_measurement_iter end,
	double& axis_inclination, double& axis_ascension, double& angular_velocity
)
{
	auto iter = end;
	auto f = make_forecast<linear_motion_model>(mp, (--iter)->jd);
	auto norm = normal(*beg, f.get_point(beg->jd).vec);
	auto jd = beg->jd;
	auto count = std::distance(++beg, end);

	double time{};
	axis_ascension = 0;
	axis_inclination = 0;
	angular_velocity = 0;

	for (; beg != end; ++beg) {
		auto angles = angles_from_quaternion(
			math::rotation(
				norm, normal(*beg, f.get_point(beg->jd).vec)
			)
		);
		double t = beg->jd - jd;
		time += math::sqr(t);
		angular_velocity += angles[0] * t;
		axis_inclination += angles[1];
		axis_ascension += angles[2];
	}

	axis_ascension /= count;
	axis_inclination /= count;
	angular_velocity /= time;
}

