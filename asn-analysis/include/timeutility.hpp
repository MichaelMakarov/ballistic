#pragma once
#include <chrono>

using clock_type = std::chrono::system_clock;
using time_point_t = clock_type::time_point;

/**
 * @brief Вычисление номера дня в году.
 *
 * @param t кол-во секунд от начала 1970 года
 * @return unsigned
 */
unsigned day_of_year(time_t t);

/**
 * @brief Формат строкового представления времени
 *
 */
enum class parse_format
{
    short_format,
    long_format
};

/**
 * @brief Чтение времени из строки.
 *
 * @tparam fmt формат предстваления
 * @param str строка
 * @return time_point_t
 */
template <parse_format fmt>
time_point_t parse_from_str(std::string const &str);
/**
 * @brief Запись в поток.
 *
 * @param os поток
 * @param t время
 */
void write_to_stream(std::ostream &os, time_point_t t);
