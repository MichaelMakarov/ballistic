#pragma once
#include <geometry.hpp>
#include <measurement.hpp>
#include <vector>
#include <string_view>

/**
 * @brief Объект конфигурации приложения
 *
 */
class application_configurer
{
    std::vector<motion_measurement> _motion_measurements;
    std::vector<rotation_measurement> _rotation_measurements;
    std::vector<geometry> _geometries;
    std::string _computation_filepath;
    std::vector<motion_measurement>::const_iterator _mbegin, _mend;
    std::vector<rotation_measurement>::const_iterator _rbegin, _rend;

public:
    application_configurer(std::string_view configpath);
    void compute() const;

private:
    void set_iterators();
};
