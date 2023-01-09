#pragma once
#include <string_view>

using seconds_t = double;
/**
 * @brief Время
 *
 */
struct time_h
{
	/**
	 * @brief Кол-во микросекунд, прошедших с 1900
	 *
	 */
	int64_t mcs;

	time_h &operator+=(seconds_t);
	time_h &operator-=(seconds_t);
};
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
};
/**
 * @brief Приведение к календарному виду
 */
calendar time_to_calendar(time_h t);
/**
 * @brief Создание по календарю
 */
time_h make_time(const calendar &c);
/**
 * @brief Текущее системное время
 */
time_h current_time();
/**
 * @brief Чтение из строки согласно указанному формату
 */
time_h make_time(std::string_view str, std::string_view fmt = "Y-m-d H:M:S.fff");

constexpr inline time_h from_seconds(int64_t sec)
{
	return time_h{.mcs = sec * 1'000'000};
}

time_h operator+(time_h, seconds_t);
time_h operator-(time_h, seconds_t);
seconds_t operator-(time_h, time_h);
bool operator<(time_h, time_h);
bool operator>(time_h, time_h);
bool operator==(time_h, time_h);
bool operator<=(time_h, time_h);
bool operator>=(time_h, time_h);
bool operator!=(time_h, time_h);

std::ostream &operator<<(std::ostream &os, time_h t);

std::ostream &operator<<(std::ostream &os, calendar const &c);
