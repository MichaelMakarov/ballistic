#include <json_utils.h>
#include <fstream>
#include <vector>
#include <array>
#include <wjson/json.hpp>
#include <wjson/strerror.hpp>

struct observatory_json {
	JSON_NAME2(n_id, "ID")
	JSON_NAME2(n_x, "X")
	JSON_NAME2(n_y, "Y")
	JSON_NAME2(n_z, "Z")
	JSON_NAME2(n_latitude, "latitude")
	JSON_NAME2(n_longitude, "longitude")
	JSON_NAME2(n_altitude, "altitude")

	using type = wjson::object<
		observatory,
		wjson::member_list<
			wjson::member<n_id, observatory, std::string, &observatory::id>,
			wjson::member<n_latitude, observatory, double, &observatory::latitude>,
			wjson::member<n_longitude, observatory, double, &observatory::longitude>,
			wjson::member<n_altitude, observatory, double, &observatory::altitude>,
			wjson::member<n_x, observatory, double, &observatory::x>,
			wjson::member<n_y, observatory, double, &observatory::y>,
			wjson::member<n_z, observatory, double, &observatory::z>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

struct observatory_list_json {
	JSON_NAME2(n_list, "observatories")

	using vector_json = wjson::array<std::vector<observatory_json>>;
	using type = wjson::object<
		observatory_list,
		wjson::member_list<
			wjson::member<n_list, observatory_list, std::vector<observatory>, &observatory_list::list, vector_json>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

using measurement_json = wjson::array<std::array<wjson::value<double>, 6>>;
using orbit_json = wjson::array<std::array<wjson::value<double>, 8>>;

struct seance_json {
	JSON_NAME(date)
	JSON_NAME(time)
	JSON_NAME(observatory)
	JSON_NAME(filter)
	JSON_NAME(type)
	JSON_NAME(duration)
	JSON_NAME(n)
	JSON_NAME(min)
	JSON_NAME(max)
	JSON_NAME(hash)
	JSON_NAME(track)
	JSON_NAME(tm)
	JSON_NAME(orb)

	using measurement_list_json = wjson::array<std::vector<measurement_json>>;
	using type = wjson::object<
		seance,
		wjson::member_list<
			wjson::member<n_date, seance, std::string, &seance::date>,
			wjson::member<n_time, seance, std::string, &seance::time>,
			wjson::member<n_observatory, seance, std::string, &seance::observatory>,
			wjson::member<n_filter, seance, std::string, &seance::filter>,
			wjson::member<n_type, seance, std::string, &seance::type>,
			wjson::member<n_duration, seance, double, &seance::duration>,
			wjson::member<n_n, seance, std::size_t, &seance::n>,
			wjson::member<n_min, seance, double, &seance::min>,
			wjson::member<n_max, seance, double, &seance::max>,
			wjson::member<n_hash, seance, std::size_t, &seance::hash>,
			wjson::member<n_track, seance, std::vector<std::array<double, 6>>, &seance::track, measurement_list_json>,
			wjson::member<n_tm, seance, std::string, &seance::tm>,
			wjson::member<n_orb, seance, std::array<double, 8>, &seance::orb, orbit_json>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

struct observation_json {
	JSON_NAME2(n_id, "ID")
	JSON_NAME(norad)
	JSON_NAME(kiam)
	JSON_NAME(nko)
	JSON_NAME(seances)

	using seance_list_json = wjson::array<std::vector<seance_json>>;
	using type = wjson::object<
		observation,
		wjson::member_list<
			wjson::member<n_id, observation, std::string, &observation::id>,
			wjson::member<n_norad, observation, std::size_t, &observation::norad>,
			wjson::member<n_kiam, observation, std::size_t, &observation::kiam>,
			wjson::member<n_nko, observation, std::size_t, &observation::nko>,
			wjson::member<n_seances, observation, std::vector<seance>, &observation::seances, seance_list_json>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

std::ofstream open_outfile(const std::string& filename) {
	std::ofstream fout(filename);
	if (!fout.is_open()) throw std::runtime_error("failed to open file " + filename);
	return fout;
}

std::ifstream open_infile(const std::string& filename) {
	std::ifstream fin(filename);
	if (!fin.is_open()) throw std::runtime_error("failed to open file " + filename);
	return fin;
}

std::string get_content(std::istream& is) {
	is.seekg(0, std::ios::beg);
	is.seekg(0, std::ios::end);
	auto str = std::string(is.tellg(), '\0');
	is.seekg(0, std::ios::beg);
	is.read(str.data(), str.size());
	return str;
}

template<typename Js, typename Obj>
void deserialize_json(const Js& serializer, const std::string& json_str, Obj& object) {
	wjson::json_error json_error;
	serializer(object, std::begin(json_str), std::end(json_str), &json_error);
	if (json_error.status()) {
		std::stringstream sstr;
		sstr << 
			"json error: " <<
			"position " << wjson::strerror::where(json_error, std::begin(json_str), std::end(json_str)) <<
			", message " << wjson::strerror::message(json_error) <<
			", trace " << wjson::strerror::trace(json_error, std::begin(json_str), std::end(json_str));
		throw std::runtime_error(sstr.str());
	}
}

observatory_list read_observatories_from_json(const std::string& filename) {
	observatory_list observatories;
	auto fin = open_infile(filename);
	deserialize_json(observatory_list_json::serializer(), get_content(fin), observatories);
	return observatories;
}

observation read_observation_from_json(const std::string& filename){
	observation observ;
	auto fin = open_infile(filename);
	deserialize_json(observation_json::serializer(), get_content(fin), observ);
	return observ;
}
