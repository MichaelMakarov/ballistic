#include <printutils.hpp>
#include <locale>

#if defined WIN32

std::wostream &cout = std::wcout;
std::wostream &cerr = std::wcerr;

#else

auto &cout = std::cout;
auto &cerr = std::cerr;

#endif

std::wostream &operator<<(std::wostream &os, char const *str)
{
    std::string loc = std::setlocale(LC_CTYPE, nullptr);
    std::setlocale(LC_CTYPE, ".utf8");
    std::size_t size = std::strlen(str);
    wchar_t wc;
    std::mbstate_t st{};
    for (std::size_t i{}; i < size;)
    {
        i += std::mbrtowc(&wc, str + i, size - i, &st);
        os << wc;
    }
    std::setlocale(LC_CTYPE, loc.c_str());
    return os;
}

std::wostream &operator<<(std::wostream &os, std::string const &str)
{
    return os << str.c_str();
}

std::wostream &operator<<(std::wostream &os, std::string_view str)
{
    return os << str.data();
}