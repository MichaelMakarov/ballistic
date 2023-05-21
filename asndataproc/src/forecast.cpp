#include <forecast.hpp>
#include <model.hpp>

forecast make_forecast(math::vec6 const &v, time_t tn, time_t tk, double s, double c,
                       std::vector<geometry> const &geometries, rotator const &rot)
{
    std::size_t constexpr harmonics{36};
    constexpr auto step = std::chrono::seconds(30).count();
    motion_model model{harmonics, s, c, geometries, rot};
    return forecast(v, tn, tk, model, step);
}