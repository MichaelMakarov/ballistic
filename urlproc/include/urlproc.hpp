#pragma once
#include <filesystem>

namespace fs = std::filesystem;

void load_file_from_url(fs::path const &urlpath, fs::path const &filepath);