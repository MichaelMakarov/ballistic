#pragma once
#include <times.hpp>
#include <format>

template <>
struct std::formatter<calendar>
{
	constexpr auto parse(std::format_parse_context &ctx)
	{
		return std::begin(ctx);
	}
	auto format(const calendar &, std::format_context &ctx) const -> decltype(ctx.out());
};

template <>
struct std::formatter<time_h> : std::formatter<calendar>
{
	auto format(time_h t, std::format_context &ctx) const -> decltype(ctx.out());
};