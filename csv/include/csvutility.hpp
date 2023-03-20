#pragma once
#include <string>

/**
 * @brief Нахождение индекса конца поля в строке csv файле.
 *
 * @param str строка файла
 * @param begin начальный индекс
 * @param sep сепаратор
 * @return std::size_t
 */
std::size_t field_end(std::string const &str, std::size_t begin, char sep);
/**
 * @brief Извлечение из строки значения double.
 *
 * @param str строка
 * @param begin начальный индекс
 * @param end конечный индекс
 * @return double
 */
double to_double(std::string const &str, std::size_t begin, std::size_t end);
/**
 * @brief Извлечение из строки значения long long.
 *
 * @param строка
 * @param begin начальный индекс
 * @param end конечный индекс
 * @return long long
 */
long long to_long(std::string const &str, std::size_t begin, std::size_t end);
