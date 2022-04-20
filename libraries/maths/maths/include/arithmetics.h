#pragma once
#include <type_traits>
#include <limits>

namespace math {

    /// <summary>
    /// Computes the square.
    /// </summary>
    /// <typeparam name="Ty">is a type of input</typeparam>
    /// <param name="value">is input value</param>
    /// <returns>value * value</returns>
    template<typename T>
    constexpr inline auto sqr(const T& value) {
        return value * value;
    }
    /// <summary>
    /// Copmutes the cube.
    /// </summary>
    /// <typeparam name="Ty">is a type of input</typeparam>
    /// <param name="value">is input value</param>
    /// <returns>value * value * value</returns>
    template<typename T>
    constexpr inline auto cube(const T& value) {
        return value * value * value;
    }

    /// <summary>
    /// Computes an absolute value.
    /// </summary>
    /// <typeparam name="Ty">is type of input</typeparam>
    /// <param name="value">is input</param>
    /// <returns>absolute value</returns>
    template<typename T>
    constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
    abs(T value) noexcept {
        if (value < T{}) return -value;
        else return value;
    }

    /// <summary>
    /// Compares two values of floating point.
    /// </summary>
    /// <typeparam name="Ty">is value type</typeparam>
    /// <param name="x">is first input</param>
    /// <param name="y">is second input</param>
    /// <param name="eps">is epsilon to compare with</param>
    /// <returns>true if two values are most equal</returns>
    template<typename T>
    constexpr typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    is_equal(T x, T y, T eps = std::numeric_limits<T>::epsilon()) {
        auto diff = math::abs(x - y);
        return diff < eps * math::abs(x + y) || diff < std::numeric_limits<T>::min();
    }

    /// <summary>
    /// Returns signum of input.
    /// </summary>
    /// <param name="value">is input value</param>
    /// <returns></returns>
    template<typename T>
    auto sign(T value) noexcept {
        T zero{};
        if (value > zero) return T(1);
        else if (value < zero) return T(-1);
        return zero;
    }
}