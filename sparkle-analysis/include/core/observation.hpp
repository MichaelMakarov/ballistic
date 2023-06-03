#pragma once
#include <times.hpp>
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
    time_type t;
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
    time_type t;
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
    unsigned id;
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
