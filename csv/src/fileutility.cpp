#include <fileutility.hpp>
#include <formatting.hpp>

std::ofstream open_outfile(std::string const &filename) noexcept(false)
{
    std::ofstream fout{filename};
    if (!fout.is_open())
    {
        throw_runtime_error("Failed to open file % to write in.", filename);
    }
    return fout;
}

std::ifstream open_infile(std::string const &filename) noexcept(false)
{
    std::ifstream fin{filename};
    if (!fin.is_open())
    {
        throw_runtime_error("Failed to open file % to read from.", filename);
    }
    return fin;
}