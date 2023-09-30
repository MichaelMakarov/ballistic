#pragma once

#include <maths.hpp>
#include <times.hpp>

namespace math
{
    vec6 operator*(vec6 const &left, time_t right);
    vec<42> operator*(vec<42> const &left, time_t right);
}

#include <integration.hpp>

using forecast = math::integrator<math::vec6, time_t, time_t>;

/**
 * @brief Интегрирование по базовой модели движения центра масс
 *
 * @param mp начальные параметры движения
 * @param tk конечное время
 * @return forecast
 */
forecast make_forecast(const math::vec6 &v, time_type tn, time_type tk, double s);

using forecast_var = math::integrator<math::vec<55>, time_t, time_t>;

forecast_var make_forecast(math::vec<55> const &v, time_type tn, time_type tk, double s);
