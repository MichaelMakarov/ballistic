#pragma once
#include <computation.hpp>

struct round_plane
{
    double mass;
    double square;
    double refl;
    vec3 normal;
};

void estimate_model(measuring_interval const &, time_h, basic_info &);

void estimate_model(measuring_interval const &, time_h, round_plane const &, extbasic_info &);

void estimate_model(measuring_interval const &, time_h, round_plane const &, rotator const &, extended_info &);
