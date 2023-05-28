#pragma once
#include <integration.hpp>
#include <timeutility.hpp>
#include <geometry.hpp>
#include <rotator.hpp>

using forecast = math::integrator<math::vec6, std::time_t, std::time_t>;

/**
 * @brief Формирование прогноза движения.
 *
 */
forecast make_forecast(math::vec6 const &v, time_t tn, time_t tk, double s,
                       std::vector<geometry> const &geometries, rotator const &rot);