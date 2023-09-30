#include <forecast.hpp>
#include <model.hpp>

using namespace std::chrono_literals;

std::time_t time_to_number(time_type t)
{
    return std::chrono::system_clock::to_time_t(t);
}

std::size_t constexpr harmonics{36};
constexpr std::time_t step = (30s).count();

forecast make_forecast(math::vec6 const &v, time_type tn, time_type tk, double s)
{
    motion_model model{harmonics, s};
    return forecast(v,
                    time_to_number(tn),
                    time_to_number(tk),
                    model,
                    step);
}

forecastext make_forecast(vec55 const &v, time_type tn, time_type tk, double s)
{
    motion_model model{harmonics, s};
    return forecastext(v,
                       time_to_number(tn),
                       time_to_number(tk),
                       model,
                       step);
}