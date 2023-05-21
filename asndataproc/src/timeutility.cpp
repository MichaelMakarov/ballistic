#include <timeutility.hpp>
#include <formatting.hpp>
#include <sstream>
#include <mutex>

time_t to_time_t(time_point_t t)
{
    return clock_type::to_time_t(t);
}

time_point_t from_time_t(time_t t)
{
    return clock_type::from_time_t(t);
}

std::mutex times_sync_obj;

int day_of_the_year(time_t t)
{
    std::lock_guard<std::mutex> lg{times_sync_obj};
    auto tmp = std::gmtime(&t);
    if (!tmp)
    {
        throw_runtime_error("Failed to create calendar representation for time %.", t);
    }
    return tmp->tm_yday;
}

constexpr auto date_fmt{"%F"};
constexpr auto datetime_fmt{"%F_%T"};

time_point_t parse_by_format(std::string const &str, char const *fmt)
{
    time_point_t t;
    std::stringstream sstr{str};
    if (!std::chrono::from_stream(sstr, fmt, t))
    {
        throw_invalid_argument("Invalid string % to parse time from using format %.", str, fmt);
    }
    return t;
}

template <>
time_point_t parse_from_str<parse_format::short_format>(std::string const &str)
{
    return parse_by_format(str, date_fmt);
}

template <>
time_point_t parse_from_str<parse_format::long_format>(std::string const &str)
{
    return parse_by_format(str, datetime_fmt);
}

void write_to_stream(std::ostream &os, time_point_t t)
{
    std::format_to(std::ostream_iterator<char>{os}, "{:%F_%T}", t);
}