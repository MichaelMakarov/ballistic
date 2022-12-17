#include <times.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <unordered_map>

constexpr int microseconds{ 1'000'000 };

time_h make_time(const calendar& c)
{
	#define throw_if(cond, msg) { if (cond) throw std::invalid_argument(msg); }

	constexpr int ord_days[12]{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	constexpr int leap_days[12]{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


	auto [year, month, day, hour, min, sec, millisec] = c;
	bool leap_year = year % 400 == 0 || (!(year % 100 == 0) && year % 4 == 0);

	throw_if(month < 1 || month > 12, "месяц имеет некорректное значение");
	auto maxday = leap_year ? leap_days[month - 1] : ord_days[month - 1];
	throw_if(day < 0 || day > maxday, "день имеет некорректное значение");
	throw_if(hour < 0 || hour > 23, "час имеет некорректное значение");
	throw_if(min < 0 || min > 59, "минута имеет некорректное значение");
	throw_if(min < 0 || sec > 59, "секунда имеет некорректное значение");
	throw_if(millisec < 0 || millisec > 999, "миллисекунда имеет некорректное значение");

	tm dt{};
	dt.tm_year = year - 1900;
	dt.tm_mon = month - 1;
	dt.tm_mday = day;
	dt.tm_hour = hour;
	dt.tm_min = min;
	dt.tm_sec = sec;

	auto t = mktime(&dt);
	throw_if(t == -1, "не удалось представить календарную дату в системном виде");

	return time_h{ t * microseconds + millisec * 1000 };
}

time_h current_time()
{
	return { std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) * microseconds };
}

const std::unordered_map<char, int> id_map{
	{ 'Y', 1 }, { 'm', 2 }, { 'd', 3 },
	{ 'H', 4 }, { 'M', 5 }, { 'S', 6 }, { 'f', 7 },
	{ '.', 8 }, { ' ', 8 }, { '-', 8 }, { '/', 8 }, { ':', 8 } 
};

time_h make_time(std::string_view str, std::string_view fmt)
{
	// parsing input string
	size_t index{};
	std::vector<std::string> values(1);
	values.reserve(7);
	for (char symbol : str) {
		if (std::isdigit(symbol)) {
			values[index].push_back(symbol);
		}
		else if (!values[index].empty()) {
			++index;
			values.push_back(std::string());
		}
	}
	if (values.size() < 3 || values.size() > 7) {
		throw std::invalid_argument("Incorrect time format string.");
	}
	// parsing format string
	int sequence[7]{ 0, 1, 1, 0, 0, 0, 0 };
	std::string buf;
	int len{ 1 };
	index = 0;
	for (char sym : fmt) {
		auto id = id_map.at(sym);// get_id(sym);
		if (id == 8) buf.push_back(sym);
		else {
			if (!buf.empty()) {
				++index;
				buf.clear();
			}
			if (index >= values.size()) {
				break;
			}
			sequence[id - 1] = std::stoi(values[index]);
			if (id == 7) len = static_cast<int>(values[index].size());
		}
	}
	calendar c{};
	c.year = sequence[0];
	c.month = sequence[1];
	c.day = sequence[2];
	c.hour = sequence[3];
	c.minute = sequence[4];
	c.second = sequence[5];
	c.millisecond = sequence[6];
	return make_time(c);
}

time_h& time_h::operator+=(seconds_t s)
{
	mcs += static_cast<long long>(s * microseconds);
	return *this;
}

time_h& time_h::operator-=(seconds_t s)
{
	return (*this) += (-s);
}

time_h operator+(time_h left, seconds_t s) {
	return left += s;
}
time_h operator-(time_h left, seconds_t s) {
	return left += (-s);
}
seconds_t operator-(time_h left, time_h right) {
	constexpr double mult{ 1.0 / microseconds };
	return (left.mcs - right.mcs) * mult;
}
bool operator<(time_h left, time_h right) {
	return left.mcs < right.mcs;
}
bool operator>(time_h left, time_h right) {
	return left.mcs > right.mcs;
}
bool operator==(time_h left, time_h right) {
	return left.mcs == right.mcs;
}
bool operator<=(time_h left, time_h right) {
	return left.mcs <= right.mcs;
}
bool operator>=(time_h left, time_h right) {
	return left.mcs >= right.mcs;
}
bool operator!=(time_h left, time_h right) {
	return left.mcs != right.mcs;
}

calendar time_to_calendar(time_h t)
{
	time_t tt{ t.mcs / microseconds };
	auto ptr = localtime(&tt);
	auto ms = static_cast<int>(std::round((t.mcs % microseconds) * 1e-3));
	calendar c{};
	c.year = ptr->tm_year + 1900;
	c.month = ptr->tm_mon + 1;
	c.day = ptr->tm_mday;
	c.hour = ptr->tm_hour;
	c.minute = ptr->tm_min;
	c.second = ptr->tm_sec + ms / 1000;
	c.millisecond = ms % 1000;
	return c;
}


