#define _CRT_SECURE_NO_WARNINGS

#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <fileutils.hpp>
#include <times.hpp>
#include <maths.hpp>
#include <observation.hpp>
#include <settings.hpp>
#include <set>

struct seance
{
	std::string date;
	std::string time;
	std::string observatory;
	std::string filter;
	std::string type;
	unsigned duration;
	unsigned count;
	double min, max;
	std::size_t hash;
	std::vector<std::array<double, 6>> track;
};

struct observation
{
	std::string id;
	unsigned norad;
	unsigned kiam;
	unsigned nko;
	std::vector<seance> seances;
};

struct observatory
{
	std::string id;
	double lat;
	double lon;
	double alt;
	double x;
	double y;
	double z;
};

struct observatories_group
{
	std::vector<observatory> observatories;
};

template <typename archive>
void serialize(archive &ar, seance &s)
{
	ar &YAS_OBJECT_NVP(
		"seance",
		("date", s.date),
		("time", s.time),
		("observatory", s.observatory),
		("filter", s.filter),
		("type", s.type),
		("duration", s.duration),
		("n", s.count),
		("min", s.min),
		("max", s.max),
		("hash", s.hash),
		("track", s.track));
}

template <typename archive>
void serialize(archive &ar, observation &o)
{
	ar &YAS_OBJECT_NVP(
		"observation",
		("ID", o.id),
		("norad", o.norad),
		("kiam", o.kiam),
		("nko", o.nko),
		("seances", o.seances));
}

template <typename archive>
void serialize(archive &ar, observatory &o)
{
	ar &YAS_OBJECT_NVP(
		"observatory",
		("ID", o.id),
		("latitude", o.lat),
		("longitude", o.lon),
		("altitude", o.alt),
		("X", o.x),
		("Y", o.y),
		("Z", o.z));
}

template <typename archive>
void serialize(archive &ar, observatories_group &g)
{
	ar &YAS_OBJECT_NVP(
		"group",
		("observatories", g.observatories));
}

yas::shared_buffer read_json_string(std::string_view filename)
{
	auto fin = open_infile(filename, std::ios_base::in | std::ios_base::ate);
	std::size_t size = fin.tellg();
	yas::shared_buffer buf(size);
	fin.seekg(0, std::ios_base::beg);
	fin.read(buf.data.get(), size);
	return buf;
}

void write_json_string(std::string const &filename, yas::shared_buffer const &buf)
{
	auto fout = open_outfile(filename, std::ios_base::out);
	fout.write(buf.data.get(), buf.size);
}

constexpr auto json_flags{yas::mem | yas::json};

template <typename T>
T read_from_json(std::string_view filename)
{
	auto buf = read_json_string(filename);
	T t;
	yas::load<json_flags>(buf, t);
	return t;
}

template <typename T>
void write_to_json(std::string const &filename, T const &t)
{
	auto buf = yas::save<json_flags>(t);
	write_json_string(filename, buf);
}

std::vector<observatory> read_observatories_from_json(std::string_view filename)
{
	return read_from_json<observatories_group>(filename).observatories;
}

observation read_observation_from_json(std::string_view filename)
{
	return read_from_json<observation>(filename);
}

namespace std
{
	template <>
	struct less<measurement_data>
	{
		bool operator()(const measurement_data &left, const measurement_data &right) const
		{
			return left.t < right.t;
		}
	};
}

std::vector<observation_seance> load_brightness_data(std::string_view obs_filename, std::string_view mes_filename)
{
	auto observatories = read_observatories_from_json(obs_filename);
	auto observation = read_observation_from_json(mes_filename);

	std::vector<observation_seance> seances;
	seances.reserve(observation.seances.size());

	for (const auto &s : observation.seances)
	{
		auto timestr = s.date + '_' + s.time;
		auto tn = parse_from_str<parse_format::long_format>(timestr.c_str());
		auto observ = std::find_if(std::begin(observatories), std::end(observatories),
								   [&s](const observatory &obs)
								   { return s.observatory == obs.id; });
		if (observ == std::end(observatories))
			continue;
		observation_seance seance{
			.id = static_cast<unsigned>(std::stoul(observ->id)),
			.o = {observ->x, observ->y, observ->z}};
		std::set<measurement_data> ranged_obs;
		for (const auto &m : s.track)
		{
			if (m[3] == 0 || m[2] == 0 || m[1] == 0)
				continue;
			ranged_obs.insert(
				measurement_data{
					.t = tn + std::chrono::milliseconds(static_cast<int>(m[0] * 1e3)),
					.a = m[1] * (math::pi / 12),
					.i = math::deg_to_rad(m[2]),
					.m = m[3],
				});
		}
		seance.m.resize(ranged_obs.size());
		std::copy(std::begin(ranged_obs), std::end(ranged_obs), std::begin(seance.m));
		seances.push_back(seance);
	}
	std::sort(std::begin(seances), std::end(seances),
			  [](observation_seance const &left, observation_seance const &right)
			  { return left.m.front().t < right.m.front().t; });

	return seances;
}

template <typename archive>
void serialize(archive &ar, project_settings &s)
{
	ar &YAS_OBJECT_NVP(
		"configuration",
		("gpt", s.gptpath),
		("tle", s.tlepath),
		("observatories", s.obspath),
		("measurements", s.mespath));
}

constexpr auto config_filename{"settings.conf"};

project_settings read_settings()
{
	return read_from_json<project_settings>(config_filename);
}

void write_settings(project_settings const &s)
{
	write_to_json<project_settings>(config_filename, s);
}
