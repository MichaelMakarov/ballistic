#include <fileutility.hpp>
#include <stdexcept>

std::ofstream open_outfile(fs::path const &filepath)
{
    std::ofstream fout{filepath};
    if (!fout.is_open())
    {
        throw std::runtime_error("Failed to open file % " + filepath.string() + " to write.");
    }
    return fout;
}

std::ifstream open_infile(fs::path const &filepath)
{
    std::ifstream fin{filepath};
    if (!fin.is_open())
    {
        throw std::runtime_error("Failed to open file " + filepath.string() + " to read.");
    }
    return fin;
}
