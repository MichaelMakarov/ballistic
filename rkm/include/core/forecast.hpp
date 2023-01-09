#pragma once
#include <integration.hpp>
#include <maths.hpp>
#include <times.hpp>

using namespace math;

/**
 * @brief Параметры движения
 *
 */
using motion_params = integratable_point<vec6, time_h>;
/**
 * @brief Прогноз движения
 *
 */
using forecast = integration_interface<vec6, time_h, double>;

class rotator;
class object_model;

/**
 * @brief Интегрирование по базовой модели движения центра масс
 *
 * @param mp начальные параметры движения
 * @param tk конечное время
 * @return forecast
 */
forecast make_forecast(const motion_params &mp, time_h tk);

/**
 * @brief Интегрирование по расширенной можели движения центра масс.
 *
 * @param mp начальные параметры движения
 * @param tk конечное время
 * @param r параметры вращения
 * @param o модель объекта
 * @return forecast
 */
forecast make_forecast(const motion_params &mp, time_h tk, const rotator &r, const object_model &o);

forecast make_forecast(motion_params const &mp, time_h tk, double s, double m);
