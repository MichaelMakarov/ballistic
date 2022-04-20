#pragma once

namespace times {

    inline constexpr double SEC_PER_MIN{ 60 };
    inline constexpr double SEC_PER_HOUR{ 3600 };
    inline constexpr double SEC_PER_DAY{ 86400 };

    inline constexpr double MILLISEC_PER_HOUR{ 3600000 };
    inline constexpr double MILLISEC_PER_MIN{ 60000 };
    inline constexpr double MILLISEC_PER_SEC{ 1000 };

    inline constexpr double HOURS_PER_DAY{ 24 };

    inline constexpr double MIN_PER_HOUR{ 60 };
    inline constexpr double MIN_PER_DAY{ 1440 };

    inline constexpr double DAYS_PER_YEAR{ 365 };

    // Julian date for 1st January 1900, 12 am
    inline constexpr double JD1900{ 2415021.5 };
    // Julian date for 1st January 2000, 12 am
    inline constexpr double JD2000{ 2451545.5 };
    // Julian date for 30th December 1899, 12 am
    inline constexpr double JD1899{ 2415019.5 };
}