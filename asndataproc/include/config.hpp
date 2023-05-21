#pragma once
#include <geometry.hpp>
#include <measurement.hpp>
#include <pathutility.hpp>
#include <vector>

/**
 * @brief Объект конфигурации приложения
 *
 */
class configurer
{
    std::vector<motion_measurement> _motion_measurements;
    std::vector<rotation_measurement> _tracker1_measurements, _tracker2_measurements;
    std::vector<geometry> _geometries;
    fs::path _computation_filepath;

public:
    configurer(fs::path const &configpath);
    std::vector<motion_measurement> const &get_motion_measurements() const { return _motion_measurements; }
    auto const &get_computationlog_filepath() const { return _computation_filepath; }
};
