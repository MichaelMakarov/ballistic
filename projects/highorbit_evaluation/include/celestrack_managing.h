#pragma once
#include <SGP4.h>
#include <structure_type.h>

#include <list>

elsetrec read_elsetrec(std::istream& is);

elsetrec read_elsetrec(std::string& first_line, std::string& second_line);

orbit_observation elsetrec_to_motion_params(elsetrec et);

std::list<elsetrec> read_tlegroup(std::istream& is);