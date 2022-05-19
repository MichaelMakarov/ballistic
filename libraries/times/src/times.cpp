#include <times.h>
#include <iomanip>
#include <vector>
#include <string>

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

time_h make_time(const std::string_view str, const std::string_view fmt)
{
	auto get_id = [](char symbol) {
		switch (symbol) {
		case 'Y': return 1;
		case 'm': return 2;
		case 'd': return 3;
		case 'H': return 4;
		case 'M': return 5;
		case 'S': return 6;
		case 'f': return 7;
		}
		return 8;
	};
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
		throw std::invalid_argument("некорректная строка времени");
	}
	// parsing format string
	int sequence[7]{ 0, 1, 1, 0, 0, 0, 0 };
	std::string buf;
	int len{ 1 };
	index = 0;
	for (char sym : fmt) {
		auto&& id = get_id(sym);
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
	return make_time(calendar{
		sequence[0], sequence[1], sequence[2],
		sequence[3], sequence[4], sequence[5], sequence[6]
	});
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

std::ostream& operator<<(std::ostream& out, time_h dt)
{
	auto ms = static_cast<int>(std::round((dt.mcs % microseconds) * 1e-3));
	time_t t{ dt.mcs / microseconds };
	if (ms == 1000) {
		++t;
		ms = 0;
	}
	auto m = localtime(&t);
	return out << std::put_time(m, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms;
}

std::ostream& operator<<(std::ostream& out, const calendar& c)
{
	#define output(w, num, sep)  << std::setfill('0') << std::setw(w) << num << sep
	return out
		output(4, c.year, '-')
		output(2, c.month, '-')
		output(2, c.day, ' ')
		output(2, c.hour, ':')
		output(2, c.minute, ':')
		output(2, c.second, '.')
		output(3, c.millisecond, "");
}

calendar time_to_calendar(time_h t)
{
	time_t tt{ t.mcs / microseconds };
	auto ptr = localtime(&tt);
	return {
		ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday,
		ptr->tm_hour, ptr->tm_min, ptr->tm_sec, (int)std::round((t.mcs % microseconds) * 1e-3)
	};
}

void stopwatch::start() {
	_finish = _start = std::chrono::steady_clock::now();
}

void stopwatch::finish() {
	_finish = std::chrono::steady_clock::now();
}

double stopwatch::duration() const {
	return std::chrono::duration<double, std::micro>(_finish - _start).count() * (1. / microseconds);
}
