#pragma once
#include <geometry.hpp>
#include <measurement.hpp>
#include <pathutility.hpp>
#include <vector>

/**
 * @brief Объект конфигурации приложения
 *
 */
class application_configurer
{
    std::vector<motion_measurement> _motion_measurements;
    std::vector<rotation_measurement> _rotation_measurements;
    std::vector<geometry> _geometries;
    fs::path _computation_filepath;
    std::vector<motion_measurement>::const_iterator _mbegin, _mend;
    std::vector<rotation_measurement>::const_iterator _rbegin, _rend;

public:
    application_configurer(fs::path const &configpath);
    void compute() const;

private:
    void set_iterators();
};
