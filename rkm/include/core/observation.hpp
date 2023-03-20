#pragma once
#include <times.hpp>
#include <string_view>
#include <vector>

/**
 * @brief Параметры движения центра масс
 *
 */
struct orbit_data
{
    /**
     * @brief Вектор состояния в ГСК (x, y, z, vx, vy, vz)
     *
     */
    double v[6];
    /**
     * @brief Время измерения
     *
     */
    time_h t;
};
/**
 * @brief Измерение параметров блеска спутника
 *
 */
struct measurement_data
{
    /**
     * @brief Время измерения
     *
     */
    time_h t;
    /**
     * @brief Прямое восхождение
     *
     */
    double a;
    /**
     * @brief Склонение
     *
     */
    double i;
    /**
     * @brief Звёздная величина блеска
     *
     */
    double m;
};
/**
 * @brief Сеанс измерений
 *
 */
struct observation_seance
{
    /**
     * @brief ID обсерватории
     *
     */
    std::string id;
    /**
     * @brief Вектор обсерватории в ГСК (x, y, z)
     *
     */
    double o[3];
    /**
     * @brief Измерения
     *
     */
    std::vector<measurement_data> m;
};

/**
 * @brief Считывание содержимого файла с орбитальными элементами.
 *
 * @param filename путь к файлу
 * @return std::vector<orbit_data>
 */
auto load_orbit_data(const std::string_view filename) -> std::vector<orbit_data>;
/**
 * @brief Считывание содержимого файлов с обсерваториями и измерениями.
 *
 * @param obs_filename путь к файлу с обсерваториями
 * @param mes_filename путь к файлу с измерениями
 * @return std::vector<observation_seance>
 */
auto load_brightness_data(const std::string_view obs_filename, const std::string_view mes_filename) -> std::vector<observation_seance>;

using orbit_iter = std::vector<orbit_data>::const_iterator;
using observ_iter = std::vector<observation_seance>::const_iterator;
