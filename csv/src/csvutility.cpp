#include <csvutility.hpp>

std::size_t field_end(std::string const &str, std::size_t begin, char sep)
{
    for (; begin != str.size(); ++begin)
    {
        if (str[begin] == sep)
            break;
    }
    return begin;
}

double to_double(std::string const &str, std::size_t begin, std::size_t end)
{
    return std::stod(str.substr(begin, end - begin));
}

long long to_long(std::string const &str, std::size_t begin, std::size_t end)
{
    return std::stoll(str.substr(begin, end - begin));
}