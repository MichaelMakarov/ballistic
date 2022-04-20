#pragma once
#include <structures.h>


struct orbit_observation {
	math::vec<6> vc;
	times::juliandate jd;
};

std::ostream& operator<< (std::ostream& out, const orbit_observation& m);

struct rotational_observation : orbit_observation {
	double magnitude;
	ball::XYZ obs;
	ball::XYZ norm;
	double inclination;
	double ascension;
};

std::ostream& operator<< (std::ostream& out, const rotational_observation& m);

#include <vector>

using orbit_observation_iter = std::vector<orbit_observation>::const_iterator;
using rotational_observation_iter = std::vector<rotational_observation>::const_iterator;