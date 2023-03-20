#include <ball.hpp>
#include <fileutility.hpp>

std::istream &operator>>(std::istream &is, potential_harmonic &p)
{
    return is >> p.cos >> p.sin;
}

void initialize_geopotential(std::string const &filename)
{
    auto fin = open_infile(filename);
    egm::read_harmonics(std::istream_iterator<potential_harmonic>{fin}, {});
}