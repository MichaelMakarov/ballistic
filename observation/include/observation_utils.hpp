#pragma once

#include <observation.hpp>

#include <string_view>

/**
 * @brief Чтение массива орбитальных данных из файла
 * 
 * @param filepath строка, содрежащая путь к файлу
 * @return std::vector<orbit_data> 
 */
std::vector<orbit_data> load_tle_observation(std::string_view filepath);

/**
 * @brief Read sparkle observation from json
 * 
 * @param obs_filename observatories file path
 * @param mes_filename measurements file path
 * @return std::vector<observation_seance> 
 */
std::vector<observation_seance> load_sparkle_observation_from_json(std::string_view obs_filename, std::string_view mes_filename);