#pragma once
#include <datetime.h>
#include <timeconstants.h>

namespace times {

	namespace utils {
		constexpr inline llong_t round_time(double value)
		{
			llong_t integer = static_cast<llong_t>(value);
			value -= integer;
			if (value > 0.5) return integer + 1;
			else if (value < -0.5) return integer - 1;
			else return integer;
		}

	}

	struct juliandate {
		llong_t day{};
		double time{};
	public:
		/// <summary>
		/// Adds seconds to current julian date.
		/// </summary>
		juliandate& operator+= (double dt) noexcept;
		/// <summary>
		/// Substracts seconds from current julian date.
		/// </summary>
		juliandate& operator-= (double dt) noexcept;
		/// <summary>
		/// Returns julian date with added number of days
		/// </summary>
		/// <param name="days"></param>
		/// <returns></returns>
		juliandate& add_days(llong_t days) noexcept;
		/// <summary>
		/// Returns julian date with added number of hours
		/// </summary>
		/// <param name="hours"></param>
		/// <returns></returns>
		juliandate& add_hours(llong_t hours) noexcept;
		/// <summary>
		/// Returns julian date with added number of minutes
		/// </summary>
		/// <param name="minutes"></param>
		/// <returns></returns>
		juliandate& add_minutes(llong_t minutes) noexcept;
		/// <summary>
		/// Returns julian date with added number of seconds
		/// </summary>
		/// <param name="seconds"></param>
		/// <returns></returns>
		juliandate& add_seconds(llong_t seconds) noexcept;
		/// <summary>
		/// Returns double representation.
		/// </summary>
		constexpr double to_double() const { return day + time; }
	};

	std::ostream& operator<< (std::ostream& os, const juliandate& jd);


	constexpr juliandate add_delta(const juliandate& jd, double delta) {
		juliandate res{ jd };
		res.time += delta;
		auto days = static_cast<llong_t>(res.time);
		res.day += days;
		res.time -= days;
		if (res.time < 0.0) {
			--res.day;
			++res.time;
		}
		return res;
	}
	constexpr bool is_equal(double left, double right) {
		auto absolute = [](double val) constexpr {
			return val < 0 ? -val : val;
		};
		return absolute(left - right) < absolute(left + right) * std::numeric_limits<double>::epsilon();
	}

	constexpr juliandate operator+ (const juliandate& jd, double dt) noexcept {
		return add_delta(jd, dt / SEC_PER_DAY);
	}
	constexpr juliandate operator- (const juliandate& jd, const double dt) noexcept {
		return add_delta(jd, -dt / SEC_PER_DAY);
	}
	constexpr double operator- (const juliandate& left, const juliandate& right) noexcept {
		return ((left.time - right.time) + (left.day - right.day)) * SEC_PER_DAY;
	}
	constexpr bool operator< (const juliandate& left, const juliandate& right) noexcept {
		return left.day < right.day || (left.day == right.day && left.time < right.time);
	}
	constexpr bool operator> (const juliandate& left, const juliandate& right) noexcept {
		return left.day > right.day || (left.day == right.day && left.time > right.time);
	}
	constexpr bool operator== (const juliandate& left, const juliandate& right) noexcept {
		return left.day == right.day && is_equal(left.time, right.time);
	}
	constexpr bool operator<= (const juliandate& left, const juliandate& right) noexcept {
		return left.day < right.day || (left.day == right.day && left.time <= right.time);
	}
	constexpr bool operator>= (const juliandate& left, const juliandate& right) noexcept {
		return left.day > right.day || (left.day == right.day && left.time >= right.time);
	}
	constexpr bool operator!= (const juliandate& left, const juliandate& right) noexcept {
		return left.day != right.day || !is_equal(left.time, right.time);
	}


	constexpr juliandate make_juliandate(llong_t day, double time)
	{
		if (time < 0 || time >= 1) throw std::invalid_argument("invalid time of day");
		return juliandate{ day, time };
	}

	constexpr inline juliandate make_juliandate(const datetime& dt)
	{
		juliandate jd{};
		llong_t
			a = (14 - dt.d.m) / 12,
			y = 4800 + dt.d.y - a,
			r = dt.d.m + 12 * a - 3;
		jd.day = dt.d.d + (153 * r + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
		jd.time = dt.t.h * (1 / 24.0) + dt.t.m * (1 / 1440.0) + (dt.t.s + dt.t.ms * 1e-3) * (1 / 86400.0);
		return jd;
	}

	constexpr inline juliandate make_juliandate(double jd) {
		llong_t days = static_cast<llong_t>(jd);
		if (jd < 0) --days;
		jd -= days;
		if (jd < 0) jd = -jd;
		return juliandate{ days, jd };
	}

	constexpr inline datetime make_datetime(const juliandate& jd)
	{
		llong_t
			a = jd.day + 32044,
			b = (4 * a + 3) / 146097,
			c = a - (146097 * b) / 4,
			d = (4 * c + 3) / 1461,
			e = c - (1461 * d) / 4,
			r = (5 * e + 2) / 153,
			year = 100 * b + d - 4800 + (r / 10),
			t = utils::round_time(jd.time * SEC_PER_DAY * 1e3);
		ushort_t
			day = static_cast<ushort_t>(e - (153 * r + 2) / 5 + 1),
			month = static_cast<ushort_t>(r + 3 - 12 * (r / 10));
		auto millisec = static_cast<ushort_t>(t % 1000);
		t /= 1000;
		auto second = static_cast<ushort_t>(t % 60);
		t /= 60;
		auto minute = static_cast<ushort_t>(t % 60);
		t /= 60;
		auto hour = static_cast<ushort_t>(t % 24);
		return datetime{ year, month, day, hour, minute, second, millisec };
	}
}