#pragma once
#include <integration.hpp>
#include <maths.hpp>
#include <times.hpp>

using namespace math;

/**
 * @brief Прогноз движения
 *
 */
using forecast = integrator<vec6, time_h, double>;

class rotator;
class object_model;

/**
 * @brief Интегрирование по базовой модели движения центра масс
 *
 * @param mp начальные параметры движения
 * @param tk конечное время
 * @return forecast
 */
forecast make_forecast(const vec6 &v, time_h tn, time_h tk);

/**
 * @brief Интегрирование по расширенной можели движения центра масс.
 *
 * @param mp начальные параметры движения
 * @param tk конечное время
 * @param r параметры вращения
 * @param o модель объекта
 * @return forecast
 */
forecast make_forecast(const vec6 &v, time_h tn, time_h tk, const rotator &r, const object_model &o);

forecast make_forecast(const vec6 &v, time_h tn, time_h tk, double s, double m);
