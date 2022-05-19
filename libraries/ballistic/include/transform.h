#pragma once

/**
 * @brief Математические координаты
 *
 */
enum struct mathematical {
	/**
	 * @brief Ортогональная
	 *
	 */
	orthogonal,
	/**
	 * @brief Сферическая
	 *
	 */
	spherical
};
/**
 * @brief Система координат
 *
 */
enum struct astronomical {
	/**
	 * @brief Абсолютная
	 *
	 */
	absolute,
	/**
	 * @brief Гринвическая
	 *
	 */
	greenwich,
	/**
	 * @brief Эклиптическая
	 *
	 */
	ecliptic
};
constexpr auto ort_cs = mathematical::orthogonal;
constexpr auto sph_cs = mathematical::spherical;
constexpr auto abs_cs = astronomical::absolute;
constexpr auto grw_cs = astronomical::greenwich;
constexpr auto ecl_cs = astronomical::ecliptic;

template<
	astronomical from_a, mathematical from_m,
	astronomical to_a, mathematical to_m>
struct transform;

/**
 * @brief Преобразование между сферической и ортогональной АСК
 */
template<>
struct transform<abs_cs, sph_cs, abs_cs, ort_cs> {
	/**
	 * @brief Преобразование из сферической в ортогональную АСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param out вектор (x, y, z)
	 */
	static void forward(const double* const in, double* const out);
	/**
	 * @brief Преобразование из ортогональной в сферическую АСК
	 *
	 * @param in вектор (x, y, z)
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double* const in, double* const out);
};
/**
 * @brief Преобразование между сферической и ортогональной ГСК
 */
template<>
struct transform<grw_cs, sph_cs, grw_cs, ort_cs> {
	/**
	 * @brief Преобразование из сферической в ортогональную ГСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param out вектор (x, y, z)
	 */
	static void forward(const double* const in, double* const out);
	/**
	 * @brief Преобразование из ортогональной в сферическую ГСК
	 *
	 * @param in вектор (x, y, z)
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void backward(const double* const in, double* const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template<>
struct transform<abs_cs, ort_cs, grw_cs, ort_cs> {
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор в АСК (x, y, z)
	 * @param t звёздное время
	 * @param out вектор в ГСК (x, y, z)
	 */
	static void forward(const double* const in, double t, double* const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор в ГСК (x, y, z)
	 * @param t звёздное время
	 * @param out вектор в АСК (x, y, z)
	 */
	static void backward(const double* const in, double t, double* const out);
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор в АСК (x, y, z, vx, vy, vz)
	 * @param t звёздное время
	 * @param out вектор в ГСК (x, y, z, vx, vy, vz)
	 */
	static void forward(const double* const in, double t, double w, double* const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор в ГСК (x, y, z, vx, vy, vz)
	 * @param t звёздное время
	 * @param out вектор в АСК (x, y, z, vx, vy, vz)
	 */
	static void backward(const double* const in, double t, double w, double* const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template<>
struct transform<abs_cs, sph_cs, grw_cs, sph_cs> {
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param t звёздное время
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double* const in, double t, double* const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param t звёздное время
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double* const in, double t, double* const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template<>
struct transform<abs_cs, sph_cs, grw_cs, ort_cs> {
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param t звёздное время
	 * @param out вектор (x, y, z)
	 */
	static void forward(const double* const in, double t, double* const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (x, y, z)
	 * @param t звёздное время
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double* const in, double t, double* const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template<>
struct transform<abs_cs, ort_cs, grw_cs, sph_cs> {
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (x, y, z)
	 * @param t звёздное время
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double* const in, double t, double* const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param t звёздное время
	 * @param out вектор (x, y, z)
	 */
	static void backward(const double* const in, double t, double* const out);
};

/**
 * @brief Преобразование между АСК и эклиптической СК
 */
template<>
struct transform<abs_cs, sph_cs, ecl_cs, sph_cs> {
	/**
	 * @brief Преобразование из АСК в эклиптическую СК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param e наклон эклиптики к экватору
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double* const in, double e, double* const out);
	/**
	 * @brief Преобразование из эклиптической СК и АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param e наклон эклиптики к экватору
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double* const in, double e, double* const out);
};
