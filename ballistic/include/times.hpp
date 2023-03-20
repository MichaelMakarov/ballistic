#pragma once

namespace std
{
	template <typename>
	struct char_traits;
	template <typename, typename>
	class basic_ostream;
	using ostream = basic_ostream<char, char_traits<char>>;
}

constexpr int milliseconds{1000};
constexpr unsigned sec_per_day{86400};
constexpr unsigned ms_per_day{milliseconds * sec_per_day};
/**
 * @brief Кол-во миллисекунд с начала 1970 г
 *
 */
using time_type = long long;

/**
 * @brief Календарное время
 *
 */
struct calendar
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int millisecond;
	int yday;

	calendar() noexcept;
	calendar(time_type t);
	calendar(int y, int m, int d, int h, int min, int s, int ms = 0) noexcept;
};
/**
 * @brief Создание из календарного времени.
 *
 * @return time_type
 */
time_type make_time(int year, int month, int day, int hour, int minute, int second, int millisec = 0);
/**
 * @brief Создание из календарного времени.
 *
 * @return time_type
 */
time_type make_time(calendar const &);
/**
 * @brief Текущее время
 *
 * @return time_ms
 */
time_type current_time();
/**
 * @brief Чтение из строки согласно указанному формату.
 * throws std::invalid_argument if invalid format string.
 */
time_type make_time(char const *str, char const *fmt = "y-M-d_h:m:s.f");

std::ostream &operator<<(std::ostream &os, calendar const &c);

constexpr double to_sec(time_type t) { return t * (1. / milliseconds); }
constexpr time_type from_sec(double t) { return static_cast<time_type>(t * milliseconds); }

constexpr time_type make_msec(time_type ms) { return ms; }
constexpr time_type make_sec(time_type sec) { return milliseconds * sec; }
constexpr time_type make_min(time_type min) { return make_sec(60) * min; }
constexpr time_type make_hour(time_type hour) { return make_min(60) * hour; }
constexpr time_type make_days(time_type days) { return make_hour(24) * days; }
