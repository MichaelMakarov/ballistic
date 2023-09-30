#include <auxiliaries.hpp>

namespace
{

	constexpr mat<2, 2> t{{0, 1}, {2, 3}};
	constexpr mat<2, 2> e{{1, 0}, {0, 1}};
	constexpr vec<2> v{1, 2};

	void _empty()
	{
		constexpr mat<3, 2> m;
		static_assert(m.rows() == 3, "Invalid rows number.");
		static_assert(m.columns() == 2, "Invalid columns number.");
		static_assert(m[0][0] == 0, "Bad initialization.");
	}

	void _init()
	{
		constexpr mat<2, 2> m{t};
		static_assert(m[0][0] == 0 && m[0][1] == 1 && m[1][0] == 2 && m[1][1] == 3, "Bad initialzation.");
	}

	void _add()
	{
		constexpr auto m = t + mat<2, 2>{{2, 1}, {1, 1}};
		static_assert(m[0][0] == 2 && m[0][1] == 2 && m[1][0] == 3 && m[1][1] == 4, "Invalid add.");
	}

	void _sub()
	{
		constexpr auto m = t - t;
		static_assert(m[0][0] == 0 && m[0][1] == 0 && m[1][0] == 0 && m[1][1] == 0, "Invalid sub.");
	}

	void _mulm()
	{
		constexpr auto m = t * e;
		static_assert(m[0][0] == t[0][0] && m[0][1] == t[0][1] && m[1][0] == t[1][0] && m[1][1] == t[1][1], "Invalid mul.");
	}

	void _mul()
	{
		constexpr auto mul{2.0};
		constexpr auto m = t * mul;
		static_assert(m[0][0] == t[0][0] * mul && m[0][1] == t[0][1] * mul && m[1][0] == t[1][0] * mul && m[1][1] == t[1][1] * mul, "Invalid mul.");
	}

	void _div()
	{
		constexpr auto mul{2.0};
		constexpr auto m = t / mul;
		static_assert(m[0][0] == t[0][0] / mul && m[0][1] == t[0][1] / mul && m[1][0] == t[1][0] / mul && m[1][1] == t[1][1] / mul, "Invalid mul.");
	}

	void _tr()
	{
		constexpr auto m = transpose(t);
		static_assert(m[0][0] == t[0][0] && m[0][1] == t[1][0] && m[1][0] == t[0][1] && m[1][1] == t[1][1], "Invalid transpose.");
	}

	void _mulv()
	{
		constexpr auto r = t * v;
		static_assert(r.size() == t.rows(), "Invalid vec size.");
		static_assert(r[0] == 2 && r[1] == 8, "Invalid vec.");
	}

	void _inv_eye()
	{
		auto m{e};
		auto d = inverse(m);
		throw_if_not(d == 1, "Determinant must be equal to 1.");
		for (size_t r{}; r < m.rows(); ++r)
		{
			throw_if_not(m[r][r] == 1, "Value must be equal to 1.");
		}
	}

	void _inv_mat()
	{
		auto m{t};
		auto d = inverse(m);
		throw_if_not(d == -2, "Determinant must be equal to -2");
		m = m * t;
		for (size_t i{}; i < m.rows(); ++i)
		{
			throw_if_not(m[i][i] == 1, "Value must be equal to 1");
		}
	}
}

void test_mat()
{
	_inv_eye();
	_inv_mat();
}