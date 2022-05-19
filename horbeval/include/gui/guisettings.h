#pragma once

constexpr inline int btn_height{ 30 };
constexpr inline int btn_width{ 100 };
constexpr inline int spacing{ 5 };
constexpr inline int timeout{ 3000 };
constexpr inline int ordinary_prec{ 3 };
constexpr inline int increased_prec{ 6 };

#include <qcommonstyle.h>

class visual_provider {
	static QCommonStyle _style;
public:
    static QIcon file_icon();
    static QIcon apply_icon();
    static QPixmap number_icon(unsigned int number);
};
