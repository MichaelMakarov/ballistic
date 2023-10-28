#pragma once

#include <string>
#include <string_view>

/**
 * @brief Параметры объекта
 * 
 */
struct object_info {
    /// @brief Масса, кг
    double mass;
    /// @brief Площадь поверхности диска, м^2
    double square;
    /// @brief Коэффициент отражения поверхности
    double refl;
};

/**
 * @brief Настройки проекта
 * 
 */
struct project_settings {
    /// @brief Путь к файлу ГПЗ
    std::string gptpath;
    /// @brief Путь к файлу TLE
    std::string tlepath;
    /// @brief Путь к файлу с обсерваториями
    std::string obspath;
    /// @brief Путь к файлу с измерениями
    std::string mespath;
};

project_settings load_project_settings_from_json(std::string_view filename);

void save_project_settings_to_json(std::string_view filename, project_settings const &settings);
