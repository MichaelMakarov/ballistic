#pragma once
#include <times.hpp>
#include <geometry.hpp>
#include <rotator.hpp>

// namespace math
// {
//     template <std::size_t size>
//     vec<size> operator*(vec<size> const &v, std::time_t t)
//     {
//         return v * (t * 1e-3);
//     }
// }

std::time_t time_to_number(time_type t);

#include <integration.hpp>

using forecast = math::integrator<math::vec6, std::time_t, std::time_t>;

/**
 * @brief Формирование прогноза движения.
 *
 */
forecast make_forecast(math::vec6 const &v, time_type tn, time_type tk, double s);

using vec55 = math::vec<55>;

using forecastext = math::integrator<vec55, std::time_t, std::time_t>;

forecastext make_forecast(vec55 const &v, time_type tn, time_type tk, double s);