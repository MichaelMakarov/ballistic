#pragma once
#include <mathconstants.h>
#include <cmath>

namespace math {
    /// <summary>
    /// Converts degrees to radians.
    /// </summary>
    /// <param name="degrees"></param>
    /// <returns>radians</returns>
    inline constexpr double deg_to_rad(double degrees) noexcept {
        return degrees * PI / 180.0;
    }
    /// <summary>
    /// Converts radians to degrees.
    /// </summary>
    /// <param name="radians"></param>
    /// <returns>degrees</returns>
    inline constexpr double rad_to_deg(double radians) noexcept {
        return radians * 180.0 / PI;
    }
    /// <summary>
    /// Converts hours to radians.
    /// </summary>
    /// <param name="hours"></param>
    /// <returns>radians</returns>
    inline constexpr double hour_to_rad(double hours) noexcept {
        return hours * PI / 12;
    }
    /// <summary>
    /// Converts radians to hours.
    /// </summary>
    /// <param name="hours"></param>
    /// <returns>hours</returns>
    inline constexpr double rad_to_hour(double rad) noexcept {
        return rad * 12 / PI;
    }

    /// <summary>
    /// Converts radians to angular seconds.
    /// </summary>
    /// <param name="radians"></param>
    /// <returns>angular seconds</returns>
    inline constexpr double rad_to_sec(double radians) noexcept {
        return rad_to_deg(radians) * SEC_PER_ROUND;
    }
    /// <summary>
    /// Converts angular seconds to radians.
    /// </summary>
    /// <param name="seconds"></param>
    /// <returns>radians</returns>
    inline constexpr double sec_to_rad(double seconds) noexcept {
        return seconds * RAD_PER_SEC;
    }

    enum class round_type {
        ZERO_DOUBLE_PI,
        MINUS_PLUS_PI
    };
    /// <summary>
    /// Adjusts angle to interval [0, 2 * pi].
    /// </summary>
    /// <param name="radians">is initial angle</param>
    /// <returns>radians</returns>
    template<round_type type = round_type::MINUS_PLUS_PI>
    double fit_to_round(double radians) {
        if (radians > PI2)  radians -= std::floor(radians / PI2) * PI2;
        else if (radians < 0) radians -= std::floor(radians / PI2) * PI2;

        if constexpr (type == round_type::MINUS_PLUS_PI) {
            if (radians > PI) radians -= PI2;
        }

        return radians;
    }
}