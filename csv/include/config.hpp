#pragma once
#include <measurement.hpp>
#include <string>
#include <vector>

class configurer
{
    std::vector<motion_measurement> _measurements;
    std::string _computation_filepath;

public:
    configurer(std::string const &filename);
    std::vector<motion_measurement> const &get_motion_measurements() const;
    std::string const &get_computationlog_filepath() const;
};
