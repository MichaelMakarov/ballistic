#include <forecast.hpp>
#include <models.hpp>

auto to_milliseconds(time_type const &t)
{
    return t.time_since_epoch().count();
}

forecast make_forecast(const math::vec6 &v, time_type tn, time_type tk)
{
    motion_model model{16};
    return forecast(v,
                    to_milliseconds(tn),
                    to_milliseconds(tk),
                    model,
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{30}).count());
}
