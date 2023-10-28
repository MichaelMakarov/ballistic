#pragma once

#include <chrono>

/// @brief Time point type
using time_type = std::chrono::sys_time<std::chrono::milliseconds>;

/**
 * @brief Вычисление номера дня в году.
 *
 * @param t кол-во секунд от начала 1970 года
 * @return unsigned
 */
unsigned day_of_year(time_t t);

/// @brief Формат строкового представления времени
enum class parse_format {
    short_format,
    long_format
};

/**
 * @brief Чтение времени из строки
 */
template <parse_format fmt>
time_type parse_from_str(char const *str);

/**
 * @brief Форматированный вывод в поток
 */
void write_to_stream(std::ostream &os, time_type t);
