#include <static_vector.h>
#include <dynamic_vector.h>

#include <gtest/gtest.h>

using namespace math;

constexpr double initvc_values[5]{
	-2.5, 1.1, 0.34, -0.65, 123.45
};

class static_vector_arithmetics_test : public ::testing::Test {
protected:
	vec<5> _vec;

	void SetUp() override;
};

class dynamic_vector_arithmetics_test : public ::testing::Test {
protected:
	vector _vec;

	void SetUp() override;
};

void static_vector_arithmetics_test::SetUp()
{
	for (size_t i{}; i < _vec.size(); ++i) _vec[i] = initvc_values[i];
}

void dynamic_vector_arithmetics_test::SetUp()
{
	_vec = vector(5);
	for (size_t i{}; i < _vec.size(); ++i) _vec[i] = initvc_values[i];
}

template<typename vector_type>
void summation(const vector_type& tmpvc) {
	auto result = tmpvc + tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] * 2);
	result = tmpvc;
	result += tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] * 2);
}

template<typename vector_type>
void substraction(const vector_type& tmpvc) {
	auto result = tmpvc - tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], 0);
	result = tmpvc;
	result -= tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], 0);
}

template<typename vector_type>
void multiplication(const vector_type& tmpvc, double mult) {
	auto result = tmpvc * mult;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] * mult);
	result = tmpvc;
	result *= mult;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] * mult);
	result = mult * tmpvc;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] * mult);
}

template<typename vector_type>
void division(const vector_type& tmpvc, double mult) {
	auto result = tmpvc / mult;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] / mult);
	result = tmpvc;
	result /= mult;
	for (size_t i{}; i < result.size(); ++i) EXPECT_DOUBLE_EQ(result[i], tmpvc[i] / mult);
}

template<typename vector_type>
void multiplication(const vector_type& tmpvc) {
	double sum{};
	for (size_t i{}; i < tmpvc.size(); ++i) sum += tmpvc[i] * tmpvc[i];
	EXPECT_DOUBLE_EQ(tmpvc * tmpvc, sum);
}

TEST_F(static_vector_arithmetics_test, summation) {
	summation(_vec);
}

TEST_F(static_vector_arithmetics_test, substraction) {
	substraction(_vec);
}

TEST_F(static_vector_arithmetics_test, multiplication_with_scalar) {
	multiplication(_vec, 2.25);
}

TEST_F(static_vector_arithmetics_test, multiplication_of_vectors) {
	multiplication(_vec);
}

TEST_F(static_vector_arithmetics_test, division) {
	division(_vec, -3.45);
}
TEST_F(dynamic_vector_arithmetics_test, summation) {
	summation(_vec);
}

TEST_F(dynamic_vector_arithmetics_test, substraction) {
	substraction(_vec);
}

TEST_F(dynamic_vector_arithmetics_test, multiplication_with_scalar) {
	multiplication(_vec, 2.25);
}

TEST_F(dynamic_vector_arithmetics_test, multiplication_of_vectors) {
	multiplication(_vec);
}

TEST_F(dynamic_vector_arithmetics_test, division) {
	division(_vec, -3.45);
}