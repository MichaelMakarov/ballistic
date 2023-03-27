#pragma once
#include <fstream>
#include <pathutility.hpp>

/**
 * @brief Открытие файла для записи.
 * May throw runtime_error.
 *
 * @param filepath путь к файлу
 * @return std::ofstream
 */
std::ofstream open_outfile(fs::path const &filepath);
/**
 * @brief Открытие файла для чтения.
 * May throw runtime_error.
 *
 * @param filepath путь к файлу
 * @return std::ifstream
 */
std::ifstream open_infile(fs::path const &filepath);