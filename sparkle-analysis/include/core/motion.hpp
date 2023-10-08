#pragma once
#include <interval.hpp>

#include <optimization.hpp>


void run_optimization(measuring_interval const &inter, orbit_data &d, math::iterations_saver &saver, std::size_t iter_count);

void run_optimization_s(measuring_interval const &inter, orbit_data &d, math::iterations_saver &saver, std::size_t iter_count);