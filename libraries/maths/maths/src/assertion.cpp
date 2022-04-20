#include <assertion.h>
#include <sstream>

computational_error::computational_error(const char* msg, const char* file, const char* func, int line) : std::runtime_error(msg)
{
	_file = file;
	_func = func;
	_line = line;
}

computational_error::computational_error(const std::string& msg, const char* file, const char* func, int line) : std::runtime_error(msg)
{
	_file = file;
	_func = func;
	_line = line;
}

std::string make_error_message(const computational_error& error)
{
	std::stringstream sstr;
	sstr << error.what() << " from file " << error.file() << ", function " << error.func() << ", line " << error.line();
	return sstr.str();
}
