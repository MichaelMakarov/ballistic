#include <linalg.h>

#include <gtest/gtest.h>

using namespace math;

template<typename vector_type>
void length_test() {
	vector_type vc{ 2.34, -1.23, 0.000023, 1e3, 0.456 };
	double len{};
	for (size_t i{}; i < vc.size(); ++i) len += vc[i] * vc[i];
	EXPECT_DOUBLE_EQ(length(vc), std::sqrt(len));
}

template<typename vector_type>
void normalize_test() {
	vector_type vc{ 2.34, -1.23, 0.000023, 1e3, 0.456 };
	double len{};
	for (size_t i{}; i < vc.size(); ++i) len += vc[i] * vc[i];
	len = std::sqrt(len);
	auto nv = vc;
	normalize(nv);
	for (size_t i{}; i < nv.size(); ++i) EXPECT_DOUBLE_EQ(nv[i], vc[i] / len);
}

TEST(static_vector, length_function) {
	length_test<vec<5>>();
}

TEST(static_vector, normalize_function) {
	normalize_test<vec<5>>();
}

TEST(static_vector, cross_function) {
	vec3 left{ 0.23, 1.34, 0.98 }, right{ 5.5, -234.56, 0.999 };
	vec3 result = cross(left, right);
	double ref[]{ 231.20746, 5.16023, -61.3188 };
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], ref[i]);
}

TEST(dynamic_vector, length_function) {
	length_test<vector>();
}

TEST(dynamic_vector, normalize_function) {
	normalize_test<vector>();
}

constexpr double initmx_values[6][6]{
	{ 1.23, -4.56, 0.123, 56.321, 100.34, 0 },
	{ 89.0, 1000, 0.2, 1e-3, 3.45, 0.987 },
	{ 23.12, -9.999, 0.012, -78.7878, 0., 0 },
	{ 0, 1.2e2, 0, 0, 1e3, 0 },
	{ 23.45, -10.101, 0.3, 0.5, 0.56, -0.00 },
	{ 1, 1, 1, 1, 1, 1 }
};
constexpr double invmx_values[6][6]{
	{ 0.09638094964021943,	 0.0019099751005200196,	 0.06863705604311958,	-0.009654708100233783,	-0.03725117024718392,	-0.0018851454242132576 },
	{-0.014836056004074349,  0.000734798002524456,	-0.01056114774817316,	 0.0014821176565482665,  0.008432848964886568,	-0.000725245628491638 },
	{-8.084854365700323,	-0.12513808070299273,	-5.736282122081676,		 0.8078748963816674,	 6.549296009776216,		 0.12351128565385394 },
	{ 0.02893411197264311,	 0.0004481623059451444,  0.007915619760748855,	-0.002898190417407914,	-0.011003931583284204,	-0.00044233619596785727 },
	{ 0.0017803267204889223,-8.817576030293472e-05,  0.0012673377297807795,  0.000822145881214208,	-0.0010119418757863884,  8.702947541899655e-05 },
	{ 7.9725950333710465,	 0.12213332105430605,	 5.6690232562962,		-0.7976262614017882,	-6.508461815034848,		 0.8794544121193998 }
};
constexpr double eps{ 1e-13 };

TEST(static_matrix, inverse_function) {
	mat<6, 6> mx;
	for (size_t r{}; r < mx.rows(); ++r) {
		for (size_t c{}; c < mx.columns(); ++c) {
			mx(r, c) = initmx_values[r][c];
		}
	}
	auto inv = inverse(mx);
	for (size_t r{}; r < inv.rows(); ++r) {
		for (size_t c{}; c < inv.columns(); ++c) {
			double diff = std::fabs(inv(r, c) - invmx_values[r][c]);
			EXPECT_LE(diff, eps);
		}
	}
}

TEST(dynamic_matrix, inverse_function) {
	matrix mx(6, 6);
	for (size_t r{}; r < mx.rows(); ++r) {
		for (size_t c{}; c < mx.columns(); ++c) {
			mx(r, c) = initmx_values[r][c];
		}
	}
	double det{};
	inverse(mx, &det);
	for (size_t r{}; r < mx.rows(); ++r) {
		for (size_t c{}; c < mx.columns(); ++c) {
			double diff = std::fabs(mx(r, c) - invmx_values[r][c]);
			EXPECT_LE(diff, eps);
		}
	}
	EXPECT_LE(std::fabs(1 - -238433328.98032677 / det), eps);
}