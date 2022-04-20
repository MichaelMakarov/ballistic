#pragma once

namespace math {
    
    inline constexpr double PI{ 3.1415926535897932 };
    inline constexpr double PI1_6{ PI / 6 };
    inline constexpr double PI1_4{ PI * 0.25 };
    inline constexpr double PI1_3{ PI / 3 };
	inline constexpr double PI1_2{ PI * 0.5 };
    inline constexpr double PI2_3{ PI * 2.0 / 3 };
    inline constexpr double PI5_6{ PI * 5.0 / 6 };
	inline constexpr double PI3_2{ PI * 1.5 };
	inline constexpr double PI2{ PI * 2 };

    // seconds per 360 degrees
    inline constexpr double SEC_PER_ROUND{ 1296000 };
    // radians per angle second
    inline constexpr double RAD_PER_SEC{ PI2 / SEC_PER_ROUND };
}