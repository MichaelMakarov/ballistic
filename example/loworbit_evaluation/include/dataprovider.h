#pragma once
#include <datetime.h>
#include <string>
#include <map>
#include <array>

/// <summary>
/// Stores space weather parameters, i.e. solar activity parameters
/// </summary>
struct space_weather {
	std::array<double, 8> Kp;
	std::array<double, 8> Ap;
	double Kpsum, Apavg, F10_7, F81;
};

std::map<times::datetime, space_weather> load_spaceweather_data(const std::string& filepath);