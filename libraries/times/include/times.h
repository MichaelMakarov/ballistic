#pragma once
#include <chrono>
#include <ostream>

using seconds_t = double;
/**
 * @brief Время
 *
 */
struct time_h {
	/**
	 * @brief Кол-во микросекуда, прошедших с 1900
	 *
	 */
	long long mcs;

	time_h& operator+=(seconds_t);
	time_h& operator-=(seconds_t);
};
/**
 * @brief Календарное время
 *
 */
struct calendar {
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
time_h make_time(const calendar& c);
/**
 * @brief Текущее системное время
 */
time_h current_time();
/**
 * @brief Чтение из строки согласно указанному формату
 */
time_h make_time(const std::string_view str, const std::string_view fmt = "Y-m-d H:M:S.fff");

time_h operator+(time_h , seconds_t );
time_h operator-(time_h , seconds_t );
seconds_t operator-(time_h , time_h );
bool operator<(time_h , time_h );
bool operator>(time_h , time_h );
bool operator==(time_h , time_h );
bool operator<=(time_h , time_h );
bool operator>=(time_h , time_h );
bool operator!=(time_h , time_h );

std::ostream& operator<<(std::ostream& out, time_h t);
std::ostream& operator<<(std::ostream& out, const calendar& c);

/**
 * @brief Секундомер
 *
 */
class stopwatch {
	std::chrono::steady_clock::time_point _start, _finish;
public:
	void start();
	void finish();
	double duration() const;
};
