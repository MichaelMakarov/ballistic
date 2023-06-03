#pragma once

#include <maths.hpp>
#include <times.hpp>

namespace math
{
    template <size_t size>
    vec<size> operator*(vec<size> const &left, time_t right)
    {
        return left * static_cast<double>(right / 1000);
    }

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
forecast make_forecast(const math::vec6 &v, time_type tn, time_type tk);
