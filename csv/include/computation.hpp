#pragma once
#include <measurement.hpp>
#include <figure.hpp>
#include <vector>
#include <string>

void compute_motion(std::vector<motion_measurement> const &measurements, std::string const &filename, figure_provider &provider);