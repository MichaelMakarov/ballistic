#pragma once
#include <stdexcept>

class computational_error : public std::runtime_error {
	const char* _file, *_func;
	int _line;
public:
	computational_error(const char* msg, const char* file, const char* func, int line);
	computational_error(const std::string& msg, const char* file, const char* func, int line);
	computational_error(const computational_error& other) = default;

	computational_error& operator= (const computational_error& other) = default;

	const char* file() const { return _file; }
	const char* func() const { return _func; }
	int line() const { return _line; }
};

std::string make_error_message(const computational_error& error);

#define ASSERT(condition, message) { if (!(condition)) throw computational_error(message, __FILE__, __func__, __LINE__); }
