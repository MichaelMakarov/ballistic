#include <fileutils.hpp>
#include <stdexcept>
#include <filesystem>

using namespace std::string_literals;

auto make_path(std::string_view filename)
{
    return std::filesystem::u8path(filename.begin(), filename.end());
}

std::ofstream open_outfile(std::string_view filename, std::ios_base::openmode mode)
{
    auto filepath = make_path(filename);
    std::ofstream fout(filepath, mode);
    if (!fout.is_open())
    {
        throw std::runtime_error(filename.data() + " is failed to open for writing."s);
    }
    return fout;
}

std::ifstream open_infile(std::string_view filename, std::ios_base::openmode mode)
{
    auto filepath = make_path(filename);
    if (!std::filesystem::exists(filepath))
    {
        throw std::runtime_error(filename.data() + " does not exist."s);
    }
    std::ifstream fin{filepath, mode};
    if (!fin.is_open())
    {
        throw std::runtime_error(filename.data() + " is failed to open for reading."s);
    }
    return fin;
}

bool exists(std::string_view filename)
{
    auto filepath = make_path(filename);
    return std::filesystem::exists(filepath);
}
