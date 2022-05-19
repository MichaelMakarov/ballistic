#pragma once

#include <type_traits>
#include <limits>

/**
 * @brief Возведение в квадрат
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return квадрат входного значения
 */
template<typename T>
constexpr inline auto sqr(const T& value) {
	return value * value;
}
/**
 * @brief Возведение в куб
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return куб входного значения
 */
template<typename T>
constexpr inline auto cube(const T& value) {
	return value * value * value;
}
/**
 * @brief Абсолютное значение
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return абсолютное значения
 */
template<typename T>
constexpr inline T cabs(T value) noexcept {
	if (value < T{}) return -value;
	else return value;
}

/**
 * @brief Сравнение двух чисел с плавающей точкой на равенство
 *
 * @tparam T тип числа
 * @param x первое число
 * @param y второе число
 * @param eps относительная точность сравнения
 * @return true, если числа эквивалентны, false - иначе
 */
template<typename T>
constexpr typename std::enable_if<std::is_floating_point_v<T>, bool>::type
is_equal(T x, T y, T eps = std::numeric_limits<T>::epsilon()) {
	auto diff = cabs(x - y);
	return diff < eps * cabs(x + y) || diff < std::numeric_limits<T>::min();
}

/**
 * @brief Сигнум числа
 *
 * @tparam T тип числа
 * @param value значение
 * @return знак числа
 */
template<typename T>
auto sign(T value) noexcept {
	T zero{};
	if (value > zero) return T(1);
	else if (value < zero) return T(-1);
	return zero;
}

/**
 * @brief Интервал значений
 *
 * @tparam T тип значения (поддреживает сравнение на меньше-равно)
 */
template<typename T>
struct interval {
	/**
	 * @brief Начальное значение на интервале
	 *
	 */
	T beg;
	/**
	 * @brief Конечное значение на интервале
	 *
	 */
	T end;

	bool operator()(T t) const {
		return beg <= t && t <= end;
	}
};

template<typename T>
interval(T beg, T end) -> interval<T>;
