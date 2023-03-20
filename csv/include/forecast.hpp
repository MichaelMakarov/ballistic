#pragma once
#include <integration.hpp>
#include <timeutility.hpp>
#include <maths.hpp>

using forecast = math::integrator<math::vec6, std::time_t, std::time_t>;

/**
 * @brief Формирование прогноза движения.
 *
 */
forecast make_forecast(math::vec6 const &v, time_t tn, time_t tk, double s, double c);