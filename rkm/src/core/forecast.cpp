#include <forecast.hpp>
#include <models.hpp>

/**
 * @brief Вычисление параметров движения
 *
 * @tparam M модель движения
 * @tparam Args доп параметры
 * @param mp нач. параметры движения
 * @param tk кон. момент времени
 * @param args параметры инициализации модели движения
 * @return прогноз
 */
template <typename M, typename... Args>
auto make_forecast(const motion_params &mp, time_h tk, const Args &...args)
{
    constexpr size_t harmonics{16};
    M model{harmonics, args...};
    return forecast(mp, tk, model, 30);
}

forecast make_forecast(const motion_params &mp, time_h tk)
{
    return make_forecast<basic_model>(mp, tk);
}

forecast make_forecast(const motion_params &mp, time_h tk, const rotator &r, const object_model &o)
{
    return make_forecast<extended_model>(mp, tk, r, o);
}

forecast make_forecast(motion_params const &mp, time_h tk, double s, double m)
{
    return make_forecast<extbasic_model>(mp, tk, s, m);
}