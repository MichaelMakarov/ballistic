#include <auxiliaries.hpp>
#include <cmath>

namespace
{
	constexpr vec<3> t{1, -2, 3};

	void _empty()
	{
		constexpr vec<4> v;
		static_assert(v.size() == 4, "Invalid size.");
		static_assert(v[0] == 0, "Invalid element.");
	}

	void _init()
	{
		constexpr auto v{t};
		static_assert(v[0] == t[0] && v[1] == t[1] && v[2] == t[2], "Invalid vec init.");
	}

	void _add()
	{
		constexpr auto v = t + t;
		static_assert(v[0] == t[0] * 2 && v[1] == t[1] * 2 && v[2] == t[2] * 2, "Invalid vec add.");
	}

	void _sub()
	{
		constexpr auto v = t - t;
		static_assert(v[0] == 0 && v[1] == 0 && v[2] == 0, "Invalid vec sub.");
	}

	void _mul()
	{
		constexpr auto v = t * 2;
		static_assert(v[0] == t[0] * 2 && v[1] == t[1] * 2 && v[2] == t[2] * 2, "Invalid vec mul.");
	}

	void _div()
	{
		constexpr auto v = t / 2;
		static_assert(v[0] == t[0] / 2 && v[1] == t[1] / 2 && v[2] == t[2] / 2, "Invalid vec mul.");
	}

	void _dot()
	{
		static_assert(t * t == 14, "Invalid mul.");
	}

	void _subv()
	{
		constexpr auto v = t.subv<0, 2>();
		static_assert(v.size() == 2, "Invalid size.");
		static_assert(v[0] == t[0] && v[1] == t[1], "Invalid subv.");
	}

	void _cross()
	{
		constexpr vec<3> v1{1, 0, 0}, v2{0, 1, 0}, v3 = cross(v1, v2);
		static_assert(v3[0] == 0 && v3[1] == 0 && v3[2] == 1, "Invalid cross.");
	}

	void _len()
	{
		throw_if_not(t.length() == std::sqrt(t * t), "Incorrect length.");
	}

}

void test_vec()
{
	_len();
}
