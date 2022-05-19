#include <optimize.h>

wrapping_logger::wrapping_logger(std::streambuf* const buf) : 
    basic_logger(std::make_unique<std::ostream>(buf))
{

}