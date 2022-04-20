#pragma once
#include <fstream>
#include <vector>
#include <array>

struct observatory {
	std::string id;
	double x, y, z;
	double latitude, longitude, altitude;
};

struct observatory_list {
	std::vector<observatory> list;
};

struct seance {
	std::string date, time, observatory, filter, type, tm;
	std::size_t n, hash;
	double duration, min, max;
	std::vector<std::array<double, 6>> track;
	std::array<double, 8> orb;
};

struct observation {
	std::string id;
	std::size_t norad, kiam, nko;
	std::vector<seance> seances;
};


std::ifstream open_infile(const std::string& filename);
std::ofstream open_outfile(const std::string& filename);


observatory_list read_observatories_from_json(const std::string& filename);

observation read_observation_from_json(const std::string& filename);

