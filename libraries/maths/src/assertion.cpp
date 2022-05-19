#include <assertion.h>

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

std::string make_error_message(const computational_error& e)
{
	return format("% ( инф. файл % ф-ция %() строка = % )", e.what(), e.file(), e.func(), e.line());
}

basic_logger::basic_logger(stream_ptr_t&& stream)
{
	_out = std::forward<stream_ptr_t>(stream);
}

basic_logger::basic_logger(basic_logger&& other) noexcept
{
    _out = std::move(other._out);
}

basic_logger& basic_logger::operator=(basic_logger&& other) noexcept
{
    _out = std::move(other._out);
    return *this;
}

basic_logger& basic_logger::operator<<(basic_logger& (*ptr) (basic_logger&))
{
	return ptr(*this);
}