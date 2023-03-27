#include <pathutility.hpp>

fs::path path_from_utf8(std::string const &pathstr)
{
    return fs::u8path(pathstr);
}