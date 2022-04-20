#include <iostream>
#include <fstream>
#include <execution>
#include <iomanip>

#include <model_wrapper.h>
#include <geometry_models.h>
#include <json_utils.h>
#include <celestrack_managing.h>

#include <optimize.h>
#include <conversion.h>

#include <stopwatch.h>

#include <solar_system.h>
#include <transform.h>


#define TRY_CATCH(expr, msg) \
	try { expr } catch (const std::exception& ex) {\
		std::cout << msg << ex.what() << std::endl;\
		return 1;\
	}

constexpr double solution_precision{ 1e-3 };

struct observation_data {
	math::vec3 pos;
	double stellar_magn;
};

std::list<elsetrec> read_tlelist(
	const std::string& filename
) {
	std::ifstream fin = open_infile(filename);
	return read_tlegroup(fin);
}

using orbit_measurement_iter = std::vector<orbit_observation>::const_iterator;


template<typename T>
concept elsetrec_iterator = std::is_same_v<typename std::iterator_traits<T>::value_type, elsetrec>;

template<elsetrec_iterator I>
std::vector<orbit_observation> elsetrecs_to_orbit_points(
	I begin, I end, 
	std::streambuf* logbuf
) {
	std::ostream logstr{ logbuf };
	std::vector<orbit_observation> points;
	points.reserve(std::distance(begin, end));

	for (size_t index{}; begin != end; ++begin) {
		std::cout << "compute " << ++index << " point..." << std::endl;
		points.push_back(elsetrec_to_motion_params(*begin));
	}

	return points;
}

void read_potential_harmonics(
	const std::string& filename
);

ball::motion_params<6> linear_motion_evaluation(
	orbit_measurement_iter begin, 
	orbit_measurement_iter end,
	std::streambuf* strbuf = nullptr
);

ball::motion_params<6> extended_motion_evaluation(
	const ball::motion_params<6>& mp, const rotation_params& rp,
	round_plane_info& info,
	orbit_measurement_iter begin, orbit_measurement_iter end,	
	std::streambuf* strbuf = nullptr
);

using observ_point_iter_t = std::vector<rotational_observation>::const_iterator;

std::vector<rotational_observation> observations_to_observ_points(
	const ball::motion_params<6>& mp,
	const ball::JD& tk,
	const std::vector<seance>& seances,
	const std::vector<observatory>& observatories
);

void compute_stellar_magnitudes(
	const ball::motion_params<13>& mp,
	const ball::JD& tk,
	const std::vector<rotational_observation>& measurements,
	double refl_square
);

void write_direction(
	const ball::motion_params<6>& mp,
	const std::vector<rotational_observation>& points
);

void eclipse_verification(
	const ball::motion_params<6>& mp,
	const std::vector<rotational_observation>& points
); 

void eclipse_verification(
	const ball::motion_params<6>& mp
);

rotation_params compute_orientation(
	const ball::motion_params<6>& mp,
	const std::vector<rotational_observation>& points
);



int main() 
{
	constexpr const char* log_filename{ "log_computation.txt" };
	constexpr const char* gm_filename{ "egm96.txt" };
	constexpr const char* observation_filename{ "X11561.json" };
	constexpr const char* observatories_filename{ "observatories.json" };
	constexpr const char* tlegroup_filename{ "sat11561.txt" }; 

	// satellite parameters
	round_plane_info satellite_info{
		.reflection = 0.21,
		.mass =	1970.0,
		.radius = 2,
		.center = { 0.1, 0.1, 0 },
		.normal = { 0, 0, 1 }
	};

	constexpr size_t points_count{ 15 };

	TRY_CATCH(
		read_potential_harmonics(gm_filename);
		, "failed to load potential harmonics: "
	);

	std::ofstream log_stream;
	TRY_CATCH(
		log_stream = open_outfile(log_filename);
		, "failed to open log file: "
	);

	std::list<elsetrec> elsetrecs;
	TRY_CATCH(
		elsetrecs = read_tlelist(tlegroup_filename);
		, "failed to load tle from file: "
	);

	std::vector<orbit_observation> orbit_points;
	decltype(auto) elsbegin = std::begin(elsetrecs), elsend = elsbegin;
	std::advance(elsend, std::min(points_count, elsetrecs.size()));

	TRY_CATCH(
		orbit_points = elsetrecs_to_orbit_points(
			elsbegin, elsend,
			log_stream.rdbuf()
		);
		, "failed to convert two line elements to motion parameters: "
	);
	std::sort(
		orbit_points.begin(), orbit_points.end(), 
		[](const auto& left, const auto& right) { return left.jd < right.jd; }
	);

	ball::motion_params<6> mp;
	TRY_CATCH(
		mp = linear_motion_evaluation(
			std::begin(orbit_points), std::end(orbit_points),
			log_stream.rdbuf()
		);
		, "failed to optimize orbit "
	);

	observation observ;
	TRY_CATCH(
		observ = read_observation_from_json(observation_filename);
		, "failed to read observation data from file: "
	);

	observatory_list observatories;
	TRY_CATCH(
		observatories = read_observatories_from_json(observatories_filename);
		, "failed to read observatories from file: "
	);

	std::vector<rotational_observation> observ_points;
	TRY_CATCH(
		observ_points = observations_to_observ_points(
			mp,
			orbit_points.back().jd, 
			observ.seances,
			observatories.list
		);
		, "failed to convert observation points: "
	);

	std::sort(
		std::execution::par,
		std::begin(observ_points), std::end(observ_points),
		[](const auto& left, const auto& right) {
			return left.jd < right.jd;
		}
	);

	/*TRY_CATCH(
		eclipse_verification(mp, observ_points);
		, "failed to check directions "
	);*/

	rotation_params rp;
	TRY_CATCH(
		rp = compute_orientation(mp, observ_points);
		, "failed to compute rotational parameters"
	);
	
	TRY_CATCH(
		mp = extended_motion_evaluation(
			mp, rp, satellite_info,
			std::begin(orbit_points), std::end(orbit_points),
			log_stream.rdbuf()
		);
		, "failed to evaluate extended motion parameters"
	);
	
 	return 0;
}

void read_potential_harmonics(
	const std::string& filename
)
{
	std::cout << "load potential harmonics..." << std::endl;
	auto fin = open_infile(filename);
	ball::load_harmonics_for<ball::egm_type::EGM96>(fin);
}

ball::motion_params<6> linear_motion_evaluation(
	orbit_measurement_iter begin, 
	orbit_measurement_iter end,
	std::streambuf* strbuf
)
{
	times::stopwatch sw;
	ball::motion_params<6> mp{};
	mp.jd = begin->jd;
	mp.loop = 1;
	mp.vec = begin->vc;

	std::ostream logout{ strbuf };
	if (strbuf) {
		logout << " ***** MEASUREMENTS ***** " << std::endl;
		size_t count{ 1 };
		for (auto it = begin; it != end; ++it, ++count) 
			logout << std::setw(6) << count << ' ' << *it << std::endl;
		logout << std::endl;
		logout << "***** INITIAL PARAMETERS OF MOTION *****" << std::endl;
		logout << mp << std::endl;
		logout << std::endl;
	}

	std::cout << "linear motion evaluation..." << std::endl;
	basic_linear_wrapper lmw(mp, begin, end);
	sw.start();
	size_t iterations = math::newton_raphson(lmw, strbuf, solution_precision);
	sw.finish();
	std::cout << "iterations count = " << iterations << " time = " << sw.duration() << std::endl;
	mp = lmw.motion_parameters();

	if (strbuf) {
		logout << " ***** IMPROVED PARAMETERS OF MOTION ***** " << std::endl;
		logout << mp << std::endl;
		logout << std::endl;
		logout << "***** FINAL RESIDUALS *****" << std::endl;

		auto f = make_forecast<linear_motion_model>(mp, lmw.finaljd());
		std::for_each(
			begin, end,
			[&f, &logout, index = 0](const auto& m) mutable {
				auto p = f.get_point(m.jd);
				logout << std::setw(6) << ++index << ' ';
				logout << "dr = " << math::length(math::slice<0, 2>(p.vec) - math::slice<0, 2>(m.vc));
				logout << std::endl;
			}
		);

		logout << std::endl;
	}

	return mp;
}

std::vector<rotational_observation> observations_to_observ_points(
	const ball::motion_params<6>& mp, const ball::JD& tk,
	const std::vector<seance>& seances,
	const std::vector<observatory>& observatories
) {
	auto interval = [tn = mp.jd, tk](const ball::JD& t){ return tn <= t && t <= tk; };

	auto forecast = make_forecast<linear_motion_model>(mp, tk);

	std::list<rotational_observation> points;

	for (const auto& s : seances) {
		auto tn = times::make_juliandate(times::make_datetime(s.date + ' ' + s.time, "y-M-d h:m:s"));
		auto tk = times::make_juliandate(times::make_datetime(s.tm));
		if (interval(tn) && interval(tk)) {
			auto obs_it = std::find_if(
				std::begin(observatories), std::end(observatories),
				[s](const observatory& obs) { return obs.id == s.observatory; }
			);
			if (obs_it == std::end(observatories)) continue;

			for (const auto& track : s.track) {
				rotational_observation p{};
				p.jd = tn + track[0];
				p.vc = math::slice<0, 5>(forecast.get_point(p.jd).vec);
				p.magnitude = track[3];
				p.obs[0] = obs_it->x;
				p.obs[1] = obs_it->y;
				p.obs[2] = obs_it->z;
				p.ascension = math::hour_to_rad(track[1]);
				p.inclination = math::deg_to_rad(track[2]);
				p.norm = normal(p.jd, p.obs, math::slice<0, 2>(p.vc));

				points.push_back(p);
			}
		}
	}

	return std::vector(points.begin(), points.end());
}

void compute_stellar_magnitudes(
	const ball::motion_params<6>& mp,
	const ball::JD& tk,
	const std::vector<rotational_observation>& measurements,
	double refl_square
) {
	auto fout = open_outfile("stellar_magnitues.txt");
	auto f = make_forecast<linear_motion_model, 6>(mp, tk);
	size_t index{ 0 };
	for (const auto& m : measurements) {
		auto p = f.get_point(m.jd);
		auto sidt = ball::sidereal_time_mean(m.jd);
		auto sun = ball::ACS_to_GCS(ball::sph_to_ort(ball::solar_model::position(m.jd)), sidt);
		double sm = apparent_magnitude<geometry_type::diffuse_sphere>(refl_square, m.obs, sun, math::slice<0, 2>(p.vec));
		fout <<
			"№ " << ++index << ' ' <<
			times::make_datetime(m.jd) << ' ' <<
			m.vc <<
			" sm ref = " << m.magnitude <<
			" sm com = " << sm <<
			std::endl;
	}
	fout.close();
}

void write_direction(
	const ball::motion_params<6>& mp,
	const std::vector<rotational_observation>& points
)
{
	auto out = open_outfile("check_directions.txt");
	auto f = make_forecast<linear_motion_model>(mp, points.back().jd);
	for (size_t i{}; i < points.size(); ++i) {
		auto& p = points[i];
		auto op = f.get_point(p.jd);
		double sidt = ball::sidereal_time_mean(op.jd);
		auto sat = ball::GCS_to_ACS(math::slice<0, 2>(op.vec), sidt);
		out << "время " << times::make_datetime(op.jd) << std::endl;
		out << 
			"рассч. сф. = " << ball::ort_to_sph(sat) <<
			" орт. = " << sat << std::endl;
		out << "изм. скл. = " << p.inclination << " восх. = " << p.ascension << std::endl;
		out <<
			"набл. сф. = " << ball::ort_to_sph(p.obs) <<
			" орт. = " << p.obs << std::endl;
		out << "солнце сф. = " << ball::solar_model::position(op.jd) << std::endl;
		out << std::endl;
	}
}

double angle_between_vectors(
	const math::vec3& left, const math::vec3& right
)
{
	return std::acos(left * right / (math::length(left) * math::length(right)));
}

void eclipse_verification(
	const ball::motion_params<6>& mp, 
	const std::vector<rotational_observation>& points
)
{
	auto out = open_outfile("eclipse_verification.txt");
	auto f = make_forecast<linear_motion_model>(mp, points.back().jd);

	for (auto& point : points) {
		auto op = f.get_point(point.jd);
		auto sun = ball::solar_position(point.jd);
		auto pos = math::slice<0, 2>(op.vec);
		double angle = math::rad_to_deg(angle_between_vectors(sun - pos, point.obs - pos)) * 0.5;
		out <<
			times::make_datetime(point.jd) << std::endl <<
			"sun sph = " << ball::ort_to_sph(sun) << " ort = " << sun << std::endl <<
			"point sph = " << ball::ort_to_sph(pos) << " ort = " << pos << std::endl <<
			"eclipse = " << eclipse_condition(sun, pos, ball::EGM96::rad) << std::endl <<
			"angle = " << angle << std::endl;
		out << std::endl;
	}
}

void eclipse_verification(
	const ball::motion_params<6>& mp
)
{
	auto jd = mp.jd;
	jd.add_days(1);

	auto out = open_outfile("eclipse_verification.txt");
	auto f = make_forecast<linear_motion_model>(mp, jd);

	for (size_t i{}; i < f.get_points().size(); ++i) {
		const auto& tp = f.get_points()[i];
		const auto& jd = f.get_times()[i];
		auto sun = ball::solar_position(jd);
		auto pos = math::slice<0, 2>(tp);
		out <<
			times::make_datetime(jd) << std::endl <<
			"sun sph = " << ball::ort_to_sph(sun) << " ort = " << sun << std::endl <<
			"point sph = " << ball::ort_to_sph(pos) << " ort = " << pos << std::endl <<
			"eclipse = " << eclipse_condition(sun, pos, ball::EGM96::rad) << std::endl;
		out << std::endl;
	}
}

rotation_params compute_orientation(
	const ball::motion_params<6>& mp, 
	const std::vector<rotational_observation>& measurements
)
{
	std::ofstream out{ "orientation.txt" };
	//out << std::setprecision(16);
	auto f = make_forecast<linear_motion_model>(mp, measurements.back().jd);

	auto compute_normal = [](const rotational_observation& o, const ball::RV& obj) {
		return normal(o.jd, o.obs, math::slice<0, 2>(obj));
	};

	const auto& start_meas = measurements.front();
	auto norm0 = compute_normal(start_meas, f.get_point(start_meas.jd).vec);

	double asc, incl, vel;
	compute_rotation(
		mp, 
		std::begin(measurements), std::end(measurements), 
		asc, incl, vel
	);
	out <<
		"phase angular velocity = " << math::rad_to_deg(vel) <<
		" inclination = " << math::rad_to_deg(incl) <<
		" ascension = " << math::rad_to_deg(asc) <<
		std::endl;

	size_t index{};
	for (const auto& m : measurements) {
		auto p = f.get_point(m.jd);
		auto norm = compute_normal(m, p.vec);
		auto q = math::rotation(norm0, norm);
		auto [axis, angle] = math::from_quaternion(q);
		auto a = angles_from_quaternion(q);
		for (auto& e : a) e = math::rad_to_deg(e);
		out <<
			"measurement №" << std::setw(4) << ++index <<
			" " << times::make_datetime(m.jd) <<
			" phase = " << a[0] << " incl = " << a[1] << " asc = " << a[2] <<
			//" axis * n = " << norm * axis <<
			//" axis * n0 = " << norm0 * axis <<
			std::endl;
	}

	rotation_params rp;
	rp.axis = ball::sph_to_ort({ 1, incl, asc });
	rp.jd = measurements.front().jd;
	rp.vel = vel;
	rp.quat = math::rotation({ 0, 0, 1 }, norm0);
	return rp;
}

ball::motion_params<6> extended_motion_evaluation(
	const ball::motion_params<6>& mp, const rotation_params& rp,
	round_plane_info& info,
	orbit_measurement_iter begin, orbit_measurement_iter end, 
	std::streambuf* strbuf
)
{
	std::ostream logout{ strbuf };
	times::stopwatch sw;

	if (strbuf) {
		logout << " ***** MEASUREMENTS ***** " << std::endl;
		size_t count{ 1 };
		for (auto it = begin; it != end; ++it, ++count)
			logout << std::setw(6) << count << ' ' << *it << std::endl;
		logout << std::endl;
		logout << "***** INITIAL PARAMETERS OF MOTION *****" << std::endl;
		logout << mp << std::endl;
		logout << "***** INITIAL PARAMETERS OF OBJECT *****" << std::endl;
		logout << info << std::endl;
		logout << std::endl;
	}

	std::cout << "extended motion evaluation..." << std::endl;
	extended_linear_wrapper mw(mp, rp, info, begin, end);
	sw.start();
	size_t iterations = math::newton_raphson(mw, strbuf, solution_precision);
	sw.finish();
	std::cout << "iterations count = " << iterations << " time = " << sw.duration() << std::endl;
	auto cp = mw.motion_parameters();
	info = mw.object_parameters();

	if (strbuf) {
		logout << " ***** IMPROVED PARAMETERS OF MOTION ***** " << std::endl;
		logout << cp << std::endl;
		logout << " ***** IMPROVED PARAMETERS OF OBJECT MODEL ***** " << std::endl;
		logout << info << std::endl;
		logout << std::endl;
		logout << "***** FINAL RESIDUALS *****" << std::endl;

		auto obj = make_round_plane(mw.object_parameters());
		auto f = make_forecast<extended_motion_model>(cp, mw.finaljd(), rp, &obj);
		std::for_each(
			begin, end,
			[&f, &logout, index = 0](const auto& m) mutable {
				auto p = f.get_point(m.jd);
				logout << std::setw(6) << ++index << ' ';
				logout << "dr = " << math::length(math::slice<0, 2>(p.vec) - math::slice<0, 2>(m.vc));
				logout << std::endl;
			}
		);

		logout << std::endl;
	}

	return cp;
}