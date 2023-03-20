#pragma once
#include <integration.hpp>
#include <maths.hpp>
#include <times.hpp>

using forecast = math::integrator<math::vec6, time_type, time_type>;

/**
 * @brief Формирование прогноза движения.
 *
 */
forecast make_forecast(math::vec6 const &v, time_type tn, time_type tk, double s, double c);