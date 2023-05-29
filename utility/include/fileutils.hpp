#pragma once
#include <fstream>
#include <string_view>

/**
 * @brief Открытие файла для записи. May throw runtime_error.
 *
 * @param filename строка, содержащая путь к файлу
 * @param mode режим открытия
 * @return std::ofstream
 */
std::ofstream open_outfile(std::string_view filename, std::ios_base::openmode mode = std::ios_base::out);
/**
 * @brief Открытие файла для чтения. May throw runtime_error.
 *
 * @param filename строка, содержащая путь к файлу
 * @param mode режим открытия
 * @return std::ifstream
 */
std::ifstream open_infile(std::string_view filename, std::ios_base::openmode mode = std::ios_base::in);

/**
 * @brief Проверяет существование фала по указанному пути.
 *
 * @param filename путь к файлу или директории
 * @return true
 * @return false
 */
bool exists(std::string_view filename);