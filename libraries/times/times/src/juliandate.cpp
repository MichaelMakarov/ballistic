#include <juliandate.h>
#include <iomanip>

namespace times {

	juliandate& juliandate::operator+= (double delta) noexcept {
		auto jd = add_delta(*this, delta / SEC_PER_DAY);
		day = jd.day;
		time = jd.time;
		return *this;
	}

	juliandate& juliandate::operator-= (double delta) noexcept {
		auto jd = add_delta(*this, -delta / SEC_PER_DAY);
		day = jd.day;
		time = jd.time;
		return *this;
	}

	juliandate& juliandate::add_days(llong_t days) noexcept {
		day += days;
		return *this;
	}
	
	juliandate& juliandate::add_hours(llong_t hours) noexcept {
		auto jd = add_delta(*this, hours / HOURS_PER_DAY);
		day = jd.day;
		time = jd.time;
		return *this;
	}
	
	juliandate& juliandate::add_minutes(llong_t minutes) noexcept {
		auto jd = add_delta(*this, minutes / MIN_PER_DAY);
		day = jd.day;
		time = jd.time;
		return *this;
	}

	juliandate& juliandate::add_seconds(llong_t seconds) noexcept {
		auto jd = add_delta(*this, seconds / SEC_PER_DAY);
		day = jd.day;
		time = jd.time;
		return *this;
	}


	std::ostream& operator<<(std::ostream& os, const juliandate& jd) {
		os << jd.day;
		if (jd.time > 1E-3 / SEC_PER_DAY) {
			os << std::setprecision(8) << '.' << std::round(jd.time * 1e8);
		}
		return os;
	}

}