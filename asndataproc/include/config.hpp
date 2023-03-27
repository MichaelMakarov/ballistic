#pragma once
#include <measurement.hpp>
#include <pathutility.hpp>
#include <vector>

class configurer
{
    std::vector<motion_measurement> _measurements;
    fs::path _computation_filepath;

public:
    configurer(fs::path const &configpath);
    std::vector<motion_measurement> const &get_motion_measurements() const { return _measurements; }
    auto const &get_computationlog_filepath() const { return _computation_filepath; }
};
