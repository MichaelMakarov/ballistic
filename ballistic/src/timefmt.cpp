#include <timefmt.hpp>

auto std::formatter<calendar>::format(const calendar& c, std::format_context& ctx) const -> decltype(ctx.out())
{
	return std::format_to(
		ctx.out(), "{:0>4d}-{:0>2d}-{:0>2d} {:0>2d}:{:0>2d}:{:0>2d}.{:0>3d}", 
		c.year, c.month, c.day, c.hour, c.minute, c.second, c.millisecond
	);
}

auto std::formatter<time_h>::format(time_h t, std::format_context& ctx) const -> decltype(ctx.out()) {
	return std::formatter<calendar>::format(time_to_calendar(t), ctx);
}

