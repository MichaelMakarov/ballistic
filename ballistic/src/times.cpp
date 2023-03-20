#include <times.hpp>
#include <formatting.hpp>
#include <ostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

std::mutex times_sync_obj;

time_type make_time(int year, int month, int day, int hour, int minute, int second, int millisec)
{
#define throw_if(cond, msg)              \
	{                                    \
		if (cond)                        \
			throw_invalid_argument(msg); \
	}

	constexpr int ord_days[12]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	constexpr int leap_days[12]{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	bool leap_year = year % 400 == 0 || (!(year % 100 == 0) && year % 4 == 0);

	throw_if(month < 1 || month > 12, "Invalid month.");
	auto maxday = leap_year ? leap_days[month - 1] : ord_days[month - 1];
	throw_if(day < 0 || day > maxday, "Invalid day.");
	throw_if(hour < 0 || hour > 23, "Invalid hour.");
	throw_if(minute < 0 || minute > 59, "Invalid minute.");
	throw_if(second < 0 || second > 59, "Invalid second.");
	throw_if(millisec < 0 || millisec > 999, "Invalid millisecond.");

	tm dt{};
	dt.tm_year = year - 1900;
	dt.tm_mon = month - 1;
	dt.tm_mday = day;
	dt.tm_hour = hour;
	dt.tm_min = minute;
	dt.tm_sec = second;

	time_t t{};
	{
		std::lock_guard<std::mutex> lg{times_sync_obj};
		t = mktime(&dt);
	}
	throw_if(t == -1, format("Failed to create corresponding system time to %-%-%_%:%:%.", year, month, day, hour, minute, second));
	return t * milliseconds + millisec;
}

time_type make_time(const calendar &c)
{
	return make_time(c.year, c.month, c.day, c.hour, c.minute, c.second, c.millisecond);
}

time_type current_time()
{
	time_t t{};
	{
		std::lock_guard<std::mutex> lg{times_sync_obj};
		t = std::time(nullptr);
	}
	return t * milliseconds;
}

class format_character
{
	static std::unordered_map<char, std::size_t> const map;

public:
	static constexpr std::size_t ms_id{6};
	static constexpr std::size_t default_id{7};
	static std::size_t get_id(char c)
	{
		return map.contains(c) ? map.at(c) : default_id;
	}
};

std::unordered_map<char, std::size_t> const format_character::map{
	{'y', 0},
	{'M', 1},
	{'d', 2},
	{'h', 3},
	{'m', 4},
	{'s', 5},
	{'f', 6},
};

time_type make_time(char const *str, char const *fmt)
{
	// parsing input string
	size_t index{};
	std::vector<std::string> values(1);
	values.reserve(7);
	while (char symbol = *str++)
	{
		if (std::isdigit(symbol))
		{
			values[index].push_back(symbol);
		}
		else if (!values[index].empty())
		{
			++index;
			values.push_back(std::string());
		}
	}
	if (values.size() < 3 || values.size() > 7)
	{
		throw_invalid_argument("Incorrect time format string.");
	}
	// parsing format string
	int sequence[7]{0, 1, 1, 0, 0, 0, 0};
	std::string buf;
	int len{1};
	index = 0;
	while (char symbol = *fmt++)
	{
		auto id = format_character::get_id(symbol);
		if (id == format_character::default_id)
		{
			buf.push_back(symbol);
		}
		else
		{
			if (!buf.empty())
			{
				++index;
				buf.clear();
			}
			if (index >= values.size())
			{
				break;
			}
			sequence[id] = std::stoi(values[index]);
			if (id == format_character::ms_id)
			{
				len = static_cast<int>(values[index].size());
			}
		}
	}
	return make_time(sequence[0], sequence[1], sequence[2], sequence[3], sequence[4], sequence[5], sequence[6]);
}

calendar::calendar() noexcept
	: year{1900},
	  month{1},
	  day{1},
	  hour{0},
	  minute{},
	  second{},
	  millisecond{},
	  yday{-1}
{
}

calendar::calendar(int y, int m, int d, int h, int min, int s, int ms) noexcept
	: year{y},
	  month{m},
	  day{d},
	  hour{h},
	  minute{min},
	  second{s},
	  millisecond{ms},
	  yday{-1}
{
}

calendar::calendar(time_type t)
{
	tm m;
	{
		time_t p = t / milliseconds;
		std::lock_guard<std::mutex> lg{times_sync_obj};
		auto ptr = localtime(&p);
		if (!ptr)
		{
			throw_invalid_argument("invalid time input.");
		}
		m = *ptr;
	}
	year = m.tm_year + 1900;
	month = m.tm_mon + 1;
	day = m.tm_mday;
	hour = m.tm_hour;
	minute = m.tm_min;
	second = m.tm_sec;
	millisecond = t % milliseconds;
	yday = m.tm_yday;
}

std::ostream &operator<<(std::ostream &os, calendar const &c)
{
	os << std::setfill('0');
	os << std::setw(4) << c.year << '.' << std::setw(2) << c.month << '.' << std::setw(2) << c.day << '_';
	os << std::setw(2) << c.hour << ':' << std::setw(2) << c.minute << ':' << std::setw(2) << c.second << '.' << std::setw(3) << c.millisecond;
	return os;
}
