#include <ball.hpp>
#include <fileutils.hpp>
#include <iostream>

std::istream &operator>>(std::istream &is, potential_harmonic &p)
{
    return is >> p.cos >> p.sin;
}

void read_geopotential(std::string_view filename)
{
    std::cout << "Reading geopotential data from " << filename << std::endl;
    auto fin = open_infile(filename);
    egm::read_harmonics(std::istream_iterator<potential_harmonic>{fin}, {});
}