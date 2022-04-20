#include <datetime.h>
#include <iomanip>
#include <chrono>
#include <vector>

namespace times {
	const char zero = '0';

	std::ostream& operator<<(std::ostream& os, const datetime& datetime) {
		os <<
			std::setfill(zero) << std::setw(4) << datetime.d.y << '.' <<
			std::setfill(zero) << std::setw(2) << datetime.d.m << '.' <<
			std::setfill(zero) << std::setw(2) << datetime.d.d << " ";
		double integer, decimal;
		decimal = std::modf(std::round(datetime.t.s * 1e3 + datetime.t.ms) * 1e-3, &integer);
		return os <<
			std::setfill(zero) << std::setw(2) << datetime.t.h << ':' <<
			std::setfill(zero) << std::setw(2) << datetime.t.m << ':' <<
			std::setfill(zero) << std::setw(2) << integer << '.' <<
			std::setfill(zero) << std::setw(3) << decimal * 1e3;

	}
	std::istream& operator>>(std::istream& is, datetime& datetime) {
		return is >> 
			datetime.d.y >> datetime.d.m >> datetime.d.d >> 
			datetime.t.h >> datetime.t.m >> datetime.t.s >> datetime.t.ms;
	}

	datetime make_datetime(const std::string& str, const std::string& format) {
		auto get_id = [](char symbol) {
			switch (symbol) {
			case 'y': return 1ull;
			case 'M': return 2ull;
			case 'd': return 3ull;
			case 'h': return 4ull;
			case 'm': return 5ull;
			case 's': return 6ull;
			case 'f': return 7ull;
			}
			return 8ull;
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
			throw std::runtime_error("invalid string");
		}

		// parsing format string

		size_t sequence[7]{ 0, 1, 1, 0, 0, 0, 0 };
		std::string buf;
		int len{ 1 };
		index = 0ull;
		for (char symbol : format) {
			auto&& id = get_id(symbol);
			if (id == 8ull) buf.push_back(symbol);
			else {
				if (!buf.empty()) {
					++index;
					buf.clear();
				}
				if (index >= values.size()) {
					break;
				}
				char* end;
				sequence[id - 1] = std::strtoull(values[index].c_str(), &end, 10);
				if (id == 7ull) len = values[index].size();
			}
		}

		return datetime{
			llong_t(sequence[0]),
			ushort_t(sequence[1]),
			ushort_t(sequence[2]),
			ushort_t(sequence[3]), 
			ushort_t(sequence[4]), 
			ushort_t(sequence[5]), 
			ushort_t(sequence[6] * std::pow(10, 3 - len))
		};
	}

	datetime datetime::now() {
		auto clock_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto time_ptr = std::localtime(&clock_time);
		return datetime{
			llong_t{ 1900 } + time_ptr->tm_year, 
			ushort_t(time_ptr->tm_mon + 1),
			ushort_t(time_ptr->tm_mday),
			ushort_t(time_ptr->tm_hour),
			ushort_t(time_ptr->tm_min), 	
			ushort_t(time_ptr->tm_sec)
		};
	}
}