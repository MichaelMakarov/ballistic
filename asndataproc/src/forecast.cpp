#include <forecast.hpp>
#include <model.hpp>

template <>
struct math::timestep_converter<time_t>
{
    static double convert(time_t t) { return static_cast<double>(t); }
};

forecast make_forecast(math::vec6 const &v, time_t tn, time_t tk, double s, double c)
{
    std::size_t constexpr harmonics{36};
    constexpr auto step = std::chrono::seconds(30).count();
    motion_model model{harmonics, s, c, nullptr, 0};
    return forecast(v, tn, tk, model, step);
}