#include <static_matrix.h>
#include <dynamic_matrix.h>

#include <gtest/gtest.h>

using namespace math;

constexpr double initmx_values[4][5]{
	{ 1, 2, 3, 4, 5 },
	{ 4, 3, 2, 1, 0 },
	{ 0.5, 0.25, 0.125, 0.0625, 0.00135 },
	{ 1, -1, 1, -1, 1 }
};
constexpr double multmx_values[4][4]{
	{ -55.        , -20.       ,  -1.63175     ,  -3.       },
	{ -20.        , -30.       ,  -3.0625      ,  -2.       },
	{ -1.63175    , -3.0625    ,  -0.3320330725,  -0.31385  },
	{ -3.         , -2.        ,  -0.31385     ,  -5.       }
};
constexpr double initvc_values[5]{
	1, 2, -100, 3, 4
};
constexpr double multvc_values[4]{
	-263, -187,  -11.3071, -100
};

class static_matrix_arithmetics_test : public ::testing::Test {
protected:
	mat<4, 5> _first;
	mat<5, 4> _second;

	void SetUp() override;
};

void static_matrix_arithmetics_test::SetUp()
{
	for (size_t i{}; i < _first.rows(); ++i) {
		for (size_t j{}; j < _first.columns(); ++j) {
			_first(i, j) = initmx_values[i][j];
			_second(j, i) = -initmx_values[i][j];;
		}
	}
}

class dynamic_matrix_arithmetics_test : public ::testing::Test {
protected:
	matrix _first, _second;

	void SetUp() override;
};

void dynamic_matrix_arithmetics_test::SetUp()
{
	_first = matrix(4, 5); 
	_second = matrix(5, 4);
	for (size_t i{}; i < _first.rows(); ++i) {
		for (size_t j{}; j < _first.columns(); ++j) {
			_first(i, j) = initmx_values[i][j];
			_second(j, i) = -initmx_values[i][j];;
		}
	}
}

template<typename matrix_type>
void summation(const matrix_type& tmpmx) {
	auto result = tmpmx + tmpmx;
	for (size_t i{}; i < tmpmx.rows(); ++i) {
		for (size_t j{}; j < tmpmx.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), initmx_values[i][j] * 2);
		}
	}

	result = tmpmx;
	result += tmpmx;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), initmx_values[i][j] * 2);
		}
	}
}

template<typename matrix_type>
void substraction(const matrix_type& tmpmx) {
	auto result = tmpmx - tmpmx;
	for (size_t i{}; i < tmpmx.rows(); ++i) {
		for (size_t j{}; j < tmpmx.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), 0);
		}
	}

	result = tmpmx;
	result -= tmpmx;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), 0);
		}
	}
}

template<typename matrix_type>
void multiplication(const matrix_type& tmpmx, double mult) {
	auto result = tmpmx * mult;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), tmpmx(i, j) * mult);
		}
	}
	result = tmpmx;
	result *= mult;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), tmpmx(i, j) * mult);
		}
	}
	result = mult * tmpmx;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), tmpmx(i, j) * mult);
		}
	}
}

template<typename matrix_type>
void division(const matrix_type& tmpmx, double mult) {
	auto result = tmpmx / mult;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), tmpmx(i, j) / mult);
		}
	}
	result = tmpmx;
	result /= mult;
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), tmpmx(i, j) / mult);
		}
	}
}

template<typename matrix_type>
void multiplication(const matrix_type& result) {
	for (size_t i{}; i < result.rows(); ++i) {
		for (size_t j{}; j < result.columns(); ++j) {
			EXPECT_DOUBLE_EQ(result(i, j), multmx_values[i][j]);
		}
	}
}

template<typename matrix_type, typename vector_type, typename O = std::enable_if_t<!std::is_arithmetic_v<vector_type>>>
void multiplication(const matrix_type& tmpmx, vector_type& tmpvc) {
	for (size_t i{}; i < tmpvc.size(); ++i) tmpvc[i] = initvc_values[i];
	auto result = tmpmx * tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], multvc_values[i]);
}

TEST_F(static_matrix_arithmetics_test, summation) {
	summation(_first);
}

TEST_F(static_matrix_arithmetics_test, substraction) {
	substraction(_first);
}

TEST_F(static_matrix_arithmetics_test, multiplication_with_scalar) {
	multiplication(_first, 2);
}

TEST_F(static_matrix_arithmetics_test, division_with_scalar) {
	division(_first, 3);
}

TEST_F(static_matrix_arithmetics_test, multiplication_of_matrices) {
	multiplication(_first * _second);
}

TEST_F(static_matrix_arithmetics_test, multiplication_with_vector) {
	vec<5> multvc;
	multiplication(_first, multvc);
}

TEST_F(dynamic_matrix_arithmetics_test, summation) {
	summation(_first);
}

TEST_F(dynamic_matrix_arithmetics_test, substraction) {
	substraction(_first);
}

TEST_F(dynamic_matrix_arithmetics_test, multiplication_with_scalar) {
	multiplication(_first, 2);
}

TEST_F(dynamic_matrix_arithmetics_test, division_with_scalar) {
	division(_first, 3);
}

TEST_F(dynamic_matrix_arithmetics_test, multiplication_of_matrices) {
	multiplication(_first * _second);
}

TEST_F(dynamic_matrix_arithmetics_test, multiplication_with_vector) {
	vector multvc(5);
	multiplication(_first, multvc);
}