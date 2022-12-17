#pragma once
#include <computation.hpp>

struct round_plane
{
    double mass;
    double square;
    double refl;
    vec3 normal;
};

void estimate_basic_model(measuring_interval const &, vec6 &, time_h);

void estimate_basic_model(measuring_interval const &, time_h, round_plane const &, basic_info &);

void estimate_extended_model(measuring_interval const &, time_h, round_plane const &, rotator const &, extended_info &);
