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
auto make_forecast(const vec6 &v, time_h tn, time_h tk, const Args &...args)
{
    constexpr size_t harmonics{16};
    M model{harmonics, args...};
    return forecast(v, tn, tk, model, make_sec(30));
}

template <>
struct timestep_converter<time_h>
{
    static double convert(time_h t) { return to_sec(t); }
};

forecast make_forecast(const vec6 &v, time_h tn, time_h tk)
{
    return make_forecast<basic_model>(v, tn, tk);
}

forecast make_forecast(const vec6 &v, time_h tn, time_h tk, const rotator &r, const object_model &o)
{
    return make_forecast<extended_model>(v, tn, tk, r, o);
}

forecast make_forecast(const vec6 &v, time_h tn, time_h tk, double s, double m)
{
    return make_forecast<extbasic_model>(v, tn, tk, s, m);
}