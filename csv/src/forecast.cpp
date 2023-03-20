#include <forecast.hpp>
#include <model.hpp>

template <>
struct math::timestep_converter<time_type>
{
    static double convert(time_type t) { return to_sec(t); }
};

forecast make_forecast(math::vec6 const &v, time_type tn, time_type tk, double s, double c)
{
    std::size_t constexpr harmonics{36};
    time_type constexpr step = make_sec(30);
    motion_model model{harmonics, s, c};
    return forecast(v, tn, tk, model, step);
}