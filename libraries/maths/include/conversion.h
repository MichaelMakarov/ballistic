#pragma once
#include <cmath>

inline constexpr double PI{ 3.1415926535897932 };
inline constexpr double PI1_6{ PI / 6 };
inline constexpr double PI1_4{ PI * 0.25 };
inline constexpr double PI1_3{ PI / 3 };
inline constexpr double PI1_2{ PI * 0.5 };
inline constexpr double PI2_3{ PI * 2.0 / 3 };
inline constexpr double PI5_6{ PI * 5.0 / 6 };
inline constexpr double PI3_2{ PI * 1.5 };
inline constexpr double PI2{ PI * 2 };

/**
 * @brief Кол-во угловых секунд в 360 градусах
 *
 */
inline constexpr double SEC_PER_ROUND{ 1296000 };
/**
 * @brief Кол-во рад в угловой секунде
 *
 */
inline constexpr double RAD_PER_SEC{ PI2 / SEC_PER_ROUND };

/**
 * @brief Преобразование градусов в радианы
 *
 * @param degrees градусы
 * @return радианы
 */
inline constexpr double deg_to_rad(double degrees) noexcept {
	return degrees * PI / 180.0;
}
/**
 * @brief Преобразование радиан в градусы
 *
 * @param radians радианы
 * @return градусы
 */
inline constexpr double rad_to_deg(double radians) noexcept {
	return radians * 180.0 / PI;
}
/**
 * @brief Преобразование часов в радианы
 *
 * @param hours часы
 * @return радианы
 */
inline constexpr double hour_to_rad(double hours) noexcept {
	return hours * PI / 12;
}
/**
 * @brief Преобразование радиан в часы
 *
 * @param rad радианы
 * @return часы
 */
inline constexpr double rad_to_hour(double rad) noexcept {
	return rad * 12 / PI;
}

/**
 * @brief Преобразование радиан в угловые секунды
 *
 * @param radians радианы
 * @return угловые секунды
 */
inline constexpr double rad_to_sec(double radians) noexcept {
	return rad_to_deg(radians) * SEC_PER_ROUND;
}
/**
 * @brief Преобразование угловых секунд в радианы
 *
 * @param seconds угловые секунды
 * @return радианы
 */
inline constexpr double sec_to_rad(double seconds) noexcept {
	return seconds * RAD_PER_SEC;
}

/**
 * @brief Интервал значений угла в рад
 *
 */
enum class round_type {
	/**
	 * @brief [   0, 2pi    ]
	 *
	 */
	zero_double_pi,
	/**
	 * @brief [ -pi, +pi    ]
	 *
	 */
	minus_plus_pi
};
/**
 * @brief Приведение радиан к интервалу значений
 *
 * @tparam type тип интервала
 * @param radians исходный угол в рад
 * @return приведённый угол в рад
 */
template<round_type type = round_type::minus_plus_pi>
double fit_to_round(double radians) {
	if (radians > PI2)  radians -= std::floor(radians / PI2) * PI2;
	else if (radians < 0) radians -= std::floor(radians / PI2) * PI2;

	if constexpr (type == round_type::minus_plus_pi) {
		if (radians > PI) radians -= PI2;
	}

	return radians;
}
