#pragma once
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Конвертирование из строки в кодировке utf8 в std::filesystem::path.
 *
 * @param pathstr
 * @return fs::path
 */
fs::path path_from_utf8(std::string const &pathstr);