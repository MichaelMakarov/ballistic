#include <wjson/wjson.hpp>

struct observatory {
    std::string id;
    double x;
    double y;
    double z;
    double lat;
    double lon;
    double alt;
};

struct observatory_list {
	std::list<observatory> list;
};

struct seance {
    std::string date;
    std::string time;
    std::string observatory;
    std::string filter;
    std::string type;
    std::string tm;
    std::size_t n;
    std::size_t hash;
    double duration;
    double min;
    double max;
    std::vector<std::array<double, 6>> track;
    std::array<double, 8> orbit;
};

struct observation {
    std::string id;
    std::size_t norad;
    std::size_t kiam;
    std::size_t nko;
    std::vector<seance> seances;
};

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
			wjson::member<n_latitude, observatory, double, &observatory::lat>,
			wjson::member<n_longitude, observatory, double, &observatory::lon>,
			wjson::member<n_altitude, observatory, double, &observatory::alt>,
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

	using list_json = wjson::list_of<observatory_json>;
	using type = wjson::object<
		observatory_list,
		wjson::member_list<
			wjson::member<n_list, observatory_list, std::list<observatory>, &observatory_list::list, list_json>
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
	JSON_NAME(orbit)

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
			wjson::member<n_orbit, seance, std::array<double, 8>, &seance::orbit, orbit_json>
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

std::string content(std::istream& in)
{
    in.seekg(0, std::ios::beg);
	in.seekg(0, std::ios::end);
	auto str = std::string(in.tellg(), '\0');
	in.seekg(0, std::ios::beg);
	in.read(str.data(), str.size());
	return str;
}

#include <assertion.h>

template<typename S, typename I, typename T>
void serialize(const S& serializer, I iter, const T& object)
{
	serializer(object, iter);
}

template<typename S, typename T>
void deserialize(const S& serializer, const std::string_view json_str, T& object)
{
    wjson::json_error error;
	serializer(object, std::begin(json_str), std::end(json_str), &error);
    ASSERT(
        !error.status(),
        format(
            "Ошибка десериализации в позиции % со следующим сообщением %",
            wjson::strerror::where(error, std::begin(json_str), std::end(json_str)),
            wjson::strerror::message(error)
        )
    );
}

std::list<observatory> observatories_from_json(std::istream& in)
{
    observatory_list observatories;
	try {
    	deserialize(observatory_list_json::serializer(), content(in), observatories);
	} catch (const std::exception& error) {
		throw std::runtime_error(format("Не удалось десериализовать обсерватории. %", error.what()));
	}
    return observatories.list;
}

observation observation_from_json(std::istream& in)
{
    observation obs;
	try {
    	deserialize(observation_json::serializer(), content(in), obs);
	} catch (const std::exception& error) {
		throw std::runtime_error(format("Не удалось десериализовать измерения. %", error.what()));
	}
    return obs;
}

#include <observation.h>

rotation_observation_provider::rotation_observation_provider(std::istream&& ostr, std::istream& mstr)
{
    auto observatories = observatories_from_json(ostr);
    auto observation = observation_from_json(mstr);

	size_t count{};
	for (const auto& s : observation.seances) {
		count += s.track.size();
	}
	_list.reserve(count);
    
	for (const auto& s : observation.seances) {
		auto tn = make_time(s.date + ' ' + s.time, "Y-m-d H:M:S");
		auto observ = std::find_if(
			std::begin(observatories), std::end(observatories),
			[&s](const observatory& obs){ return s.observatory == obs.id; }
		);
		if (observ == std::end(observatories)) continue;

		for (const auto& m : s.track) {
			rotation_observation o;
			o.t = tn + m[0];
			o.o[0] = observ->x;
			o.o[1] = observ->y;
			o.o[2] = observ->z;
			o.s = m[3];
			_list.push_back(o);
		}
	}

	std::sort(std::begin(_list), std::end(_list), [](const auto& left, const auto& right){ return left.t < right.t; });
}

#include <mainmodel.h>

struct round_plane_info_json {
	JSON_NAME(mass)
	JSON_NAME(size)
	JSON_NAME(refl)

	using type = wjson::object<
		round_plane_info,
		wjson::member_list<
			wjson::member<n_mass, round_plane_info, double, &round_plane_info::mass>,
			wjson::member<n_size, round_plane_info, double, &round_plane_info::rad>,
			wjson::member<n_refl, round_plane_info, double, &round_plane_info::refl>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

struct project_settings_json {
    JSON_NAME(gpt)
    JSON_NAME(tle)
    JSON_NAME(observatories)
    JSON_NAME(observation)
	JSON_NAME(object)

    using type = wjson::object<
		project_settings,
		wjson::member_list<
			wjson::member<n_gpt, project_settings, std::string, &project_settings::gptpath>,
			wjson::member<n_tle, project_settings, std::string, &project_settings::tlepath>,
			wjson::member<n_observatories, project_settings, std::string, &project_settings::obspath>,
			wjson::member<n_observation, project_settings, std::string, &project_settings::mespath>,
			wjson::member<n_object, project_settings, round_plane_info, &project_settings::object, round_plane_info_json>
		>
	>;
	using serializer = type::serializer;
	using target = type::target;
	using member_list = type::member_list;
};

project_settings read(std::istream&& in)
{
	project_settings p;
	deserialize(project_settings_json::serializer(), content(in), p);
	return p;
}

void write(std::ostream&& out, const project_settings& p)
{
	serialize(project_settings_json::serializer(), std::ostream_iterator<char>{ out }, p);
}