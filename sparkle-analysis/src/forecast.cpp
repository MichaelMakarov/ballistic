#include <forecast.hpp>
#include <models.hpp>

auto to_milliseconds(time_type const &t)
{
    return t.time_since_epoch().count();
}

double to_double(time_t t)
{
    return t * 1e-3;
}

namespace math
{
    vec6 operator*(vec6 const &left, time_t right)
    {
        return left * to_double(right);
    }

    vec<55> operator*(vec<55> const &left, time_t right)
    {
        return left * to_double(right);
    }
}

constexpr size_t harmonics{16};

forecast make_forecast(const math::vec6 &v, time_type tn, time_type tk, double s)
{
    motion_model model{harmonics, s};
    return forecast(v,
                    to_milliseconds(tn),
                    to_milliseconds(tk),
                    model,
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{30}).count());
}

forecast_var make_forecast(math::vec<55> const &v, time_type tn, time_type tk, double s)
{
    motion_model model{harmonics, s};
    return forecast_var(v,
                        to_milliseconds(tn),
                        to_milliseconds(tk),
                        model,
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{30}).count());
}
