#include <timeutility.hpp>
#include <sstream>
#include <stdexcept>
#include <format>

unsigned day_of_year(time_t t)
{
    std::chrono::seconds seconds{t};
    std::chrono::sys_days days{std::chrono::duration_cast<std::chrono::days>(seconds)};
    std::chrono::year_month_day ymd{days};
    int y = static_cast<int>(ymd.year());
    unsigned m = static_cast<unsigned>(ymd.month());
    unsigned d = static_cast<unsigned>(ymd.day());
    unsigned n1 = (275 * m) / 9;
    unsigned n2 = (m + 9) / 12;
    unsigned n3 = 1 + (y - 4 * (y / 4) + 2) / 3;
    unsigned n = n1 - (n2 * n3) + d - 30;
    return n;
}

constexpr auto date_fmt{"%F"};
constexpr auto datetime_fmt{"%F_%T"};

time_point_t parse_by_format(char const *str, char const *fmt)
{
    time_point_t t;
    std::stringstream sstr{str};
    if (!std::chrono::from_stream(sstr, fmt, t))
    {
        throw std::invalid_argument(std::format("Invalid string {} to parse time from using format {}.", str, fmt));
    }
    return t;
}

template <>
time_point_t parse_from_str<parse_format::short_format>(char const *str)
{
    return parse_by_format(str, date_fmt);
}

template <>
time_point_t parse_from_str<parse_format::long_format>(char const *str)
{
    return parse_by_format(str, datetime_fmt);
}

void write_to_stream(std::ostream &os, time_point_t t)
{
    std::format_to(std::ostream_iterator<char>{os}, "{:%F_%T}", t);
}