#pragma once
#include <ostream>
#include <istream>

namespace times {
	using llong_t = long long;
	using ushort_t = unsigned short;

	constexpr bool leap_year(llong_t year) noexcept {
		return year % 400 == 0 || (!(year % 100 == 0) && year % 4 == 0);
	}
	constexpr bool verify_day(llong_t year, ushort_t month, ushort_t day) {
		constexpr ushort_t ord_days[12]{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		constexpr ushort_t leap_days[12]{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (day == 0) return false;
		if (leap_year(year)) return day <= leap_days[month - 1];
		else return day <= ord_days[month - 1];
	}

	struct date {
		llong_t y{ 0 };
		ushort_t m{ 1 }, d{ 1 };
	};

	struct time {
		ushort_t h{}, m{}, s{}, ms{};
	};

	struct datetime {
		date d;
		time t;

		static datetime now();

		constexpr llong_t year() const { return d.y; }
		constexpr ushort_t month() const { return d.m; }
		constexpr ushort_t day() const { return d.d; }
		constexpr ushort_t hour() const { return t.h; }
		constexpr ushort_t minute() const { return t.m; }
		constexpr ushort_t second() const { return t.s; }
		constexpr ushort_t millisec() const { return t.ms; }
	};

	constexpr bool operator> (const date& left, const date& right) noexcept {
		if (left.y > right.y) return true;
		else if (left.y == right.y) {
			if (left.m > right.m) return true;
			else if (left.m == right.m) {
				return left.d > right.d;
			}
		}
		return false;
	}
	constexpr bool operator< (const date& left, const date& right) noexcept {
		if (left.y < right.y) return true;
		else if (left.y == right.y) {
			if (left.m < right.m) return true;
			else if (left.m == right.m) {
				return left.d < right.d;
			}
		}
		return false;
	}
	constexpr bool operator== (const date& left, const date& right) noexcept {
		return left.y == right.y && left.m == right.m && left.d == right.d;
	}
	constexpr bool operator>= (const date& left, const date& right) noexcept {
		if (left.y > right.y) return true;
		else if (left.y == right.y) {
			if (left.m > right.m) return true;
			else if (left.m == right.m) {
				return left.d >= right.d;
			}
		}
		return false;
	}
	constexpr bool operator<= (const date& left, const date& right) noexcept {
		if (left.y < right.y) return true;
		else if (left.y == right.y) {
			if (left.m < right.m) return true;
			else if (left.m == right.m) {
				return left.d <= right.d;
			}
		}
		return false;
	}
	constexpr bool operator!= (const date& left, const date& right) noexcept {
		return left.y != right.y || left.m != right.m || left.d != right.d;
	}
	constexpr bool operator> (const time& left, const time& right) noexcept {
		if (left.h > right.h) return true;
		else if (left.h == right.h) {
			if (left.m > right.m) return true;
			else if (left.m == right.m) {
				if (left.s > right.s) return true;
				else if (left.s == right.s) {
					return left.ms > right.ms;
				}
			}
		}
		return false;
	}
	constexpr bool operator< (const time& left, const time& right) noexcept {
		if (left.h < right.h) return true;
		else if (left.h == right.h) {
			if (left.m < right.m) return true;
			else if (left.m == right.m) {
				if (left.s < right.s) return true;
				else if (left.s == right.s) {
					return left.ms < right.ms;
				}
			}
		}
		return false;
	}
	constexpr bool operator== (const time& left, const time& right) noexcept {
		return left.h == right.h && left.m == right.m && left.s == right.s && left.ms == right.ms;
	}
	constexpr bool operator>= (const time& left, const time& right) noexcept {
		if (left.h > right.h) return true;
		else if (left.h == right.h) {
			if (left.m > right.m) return true;
			else if (left.m == right.m) {
				if (left.s > right.s) return true;
				else if (left.s == right.s) {
					return left.ms >= right.ms;
				}
			}
		}
		return false;
	}
	constexpr bool operator<= (const time& left, const time& right) noexcept {
		if (left.h < right.h) return true;
		else if (left.h == right.h) {
			if (left.m < right.m) return true;
			else if (left.m == right.m) {
				if (left.s < right.s) return true;
				else if (left.s == right.s) {
					return left.ms <= right.ms;
				}
			}
		}
		return false;
	}
	constexpr bool operator!= (const time& left, const time& right) noexcept {
		return left.h != right.h || left.m != right.m || left.s != right.s || left.ms != right.ms;
	}
	constexpr bool operator> (const datetime& left, const datetime& right) noexcept {
		return left.d > right.d || (left.d == right.d && left.t > right.t);
	}
	constexpr bool operator< (const datetime& left, const datetime& right) noexcept {
		return left.d < right.d || (left.d == right.d && left.t < right.t);
	}
	constexpr bool operator== (const datetime& left, const datetime& right) noexcept {
		return left.d == right.d && left.t == right.t;
	}
	constexpr bool operator>= (const datetime& left, const datetime& right) noexcept {
		return left.d > right.d || (left.d == right.d && left.t >= right.t);
	}
	constexpr bool operator<= (const datetime& left, const datetime& right) noexcept {
		return left.d < right.d || (left.d == right.d && left.t <= right.t);
	}
	constexpr bool operator!= (const datetime& left, const datetime& right) noexcept {
		return left.d != right.d || left.t != right.t;
	}

	std::ostream& operator<< (std::ostream& os, const datetime& dt);
	std::istream& operator>> (std::istream& os, datetime& dt);

	constexpr datetime make_datetime(llong_t y, ushort_t m, ushort_t d, ushort_t h, ushort_t tm, ushort_t s, ushort_t ms = 0)
	{
		datetime dt{};
		if (m == 0 || m > 12) throw std::invalid_argument("invalid month");
		if (!verify_day(y, m, d)) throw std::invalid_argument("invalid day");
		if (h > 23) throw std::invalid_argument("invalid hour");
		if (tm > 59) throw std::invalid_argument("invalid minute");
		if (s > 59) throw std::invalid_argument("invalid second");
		if (ms > 999) throw std::invalid_argument("invalid millisecond");
		dt.d.d = d;
		dt.d.m = m;
		dt.d.y = y;
		dt.t.h = h;
		dt.t.m = tm;
		dt.t.s = s;
		dt.t.ms = ms;
		return dt;
	}

	/// <summary>
	/// Parses datetime from string of specified format (may throw an exception).
	/// </summary>
	/// <param name="str">contains data</param>
	/// <param name="format">specifies format</param>
	/// <returns></returns>
	datetime make_datetime(const std::string& str, const std::string& format = "y.M.d h:m:s.f");
}