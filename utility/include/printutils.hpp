#pragma once
#include <iostream>
#include <string_view>

#if defined WIN32

extern std::wostream &cout;
extern std::wostream &cerr;

std::wostream &operator<<(std::wostream &, char const *);
std::wostream &operator<<(std::wostream &, std::string const &);
std::wostream &operator<<(std::wostream &, std::string_view);

#else

extern std::ostream &cout;
extern std::ostream &cerr;

#endif
