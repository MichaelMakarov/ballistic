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

#define ASSERT(condition, message) if (!(condition)) throw computational_error(message, __FILE__, __func__, __LINE__);

#include <ostream>
#include <memory>

using stream_ptr_t = std::unique_ptr<std::ostream>;

class basic_logger {
	stream_ptr_t _out;
public:
	basic_logger(stream_ptr_t&& stream = {});
	basic_logger(basic_logger&& other) noexcept;

	basic_logger& operator=(basic_logger&& other) noexcept;

	basic_logger& operator<<(basic_logger& (*ptr) (basic_logger&));

	template<typename T>
	friend basic_logger& operator<<(basic_logger& logger, const T& obj);
protected:
	std::ostream& out() { return *_out; }
	bool good() const { return static_cast<bool>(_out); }
};

template<typename T>
basic_logger& operator<<(basic_logger& logger, const T& obj) {
	if (logger.good()) logger.out() << obj;
	return logger;
}

namespace std {
	inline basic_logger& endl(basic_logger& logger) { return logger << '\n'; }
}

#include <sstream>

namespace internal {
	constexpr char delimiter{ '%' };

	inline void format_impl(std::stringstream& sstr, const char* const fmt) {
		sstr << fmt;
	}

	template<typename Arg, typename ... Args>
	void format_impl(std::stringstream& sstr, const char* fmt, Arg&& arg, Args&&... args) {
		for (; *fmt != '\0'; ++fmt) {
			if (*fmt == delimiter) {
				sstr << arg;
				format_impl(sstr, ++fmt, std::forward<Args>(args)...);
				return;
			} else {
				sstr << *fmt;
			}
		}
	}
}

template<typename ... Args>
std::string format(const char* const fmt, Args&&... args) {
	std::stringstream sstr;
	internal::format_impl(sstr, fmt, std::forward<Args>(args)...);
	return sstr.str();
}

template<typename ... Args>
std::string format(const std::string& fmt, Args&&... args) {
	return format(fmt.data(), std::forward<Args>(args)...);
}

