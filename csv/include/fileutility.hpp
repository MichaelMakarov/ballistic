#pragma once
#include <fstream>

/**
 * @brief Открытие файла для записи.
 * May throw runtime_error.
 *
 * @param filename путь к файлу
 * @return std::ofstream
 */
std::ofstream open_outfile(std::string const &filename) noexcept(false);
/**
 * @brief Открытие файла для чтения.
 * May throw runtime_error.
 *
 * @param filename путь к файлу
 * @return std::ifstream
 */
std::ifstream open_infile(std::string const &filename) noexcept(false);