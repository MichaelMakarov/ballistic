#pragma once
#include <string>

/**
 * @brief Параметры объекта
 * 
 */
struct object_info {
    /**
     * @brief Масса, кг
     * 
     */
    double mass;
    /**
     * @brief Площадь поверхности диска, м^2
     * 
     */
    double square;
    /**
     * @brief Коэффициент отражения поверхности
     * 
     */
    double refl;
};

/**
 * @brief Настройки проекта
 * 
 */
struct project_settings {
    /**
     * @brief Путь к файлу ГПЗ
     * 
     */
    std::string gptpath;
    /**
     * @brief Путь к файлу TLE
     * 
     */
    std::string tlepath;
    /**
     * @brief Путь к файлу с обсерваториями
     * 
     */
    std::string obspath;
    /**
     * @brief Путь к файлу с измерениями
     * 
     */
    std::string mespath;
    /**
     * @brief Параметры КА
     * 
     */
    object_info object;
};

extern project_settings settings;
