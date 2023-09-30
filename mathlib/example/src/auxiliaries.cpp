#include <auxiliaries.hpp>
#include <sstream>
#include <stdexcept>

void throw_computation_error(char const *msg, char const *file, int line)
{
    std::stringstream sout;
    sout << "file " << file << ", line " << line << ", message " << msg << std::endl;
    throw std::runtime_error(sout.str());
}