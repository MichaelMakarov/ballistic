#pragma once

/**
 * @brief Математические координаты
 *
 */
enum struct mathematical
{
	/**
	 * @brief Ортогональная
	 *
	 */
	ort,
	/**
	 * @brief Сферическая
	 *
	 */
	sph
};
/**
 * @brief Система координат
 *
 */
enum struct astronomical
{
	/**
	 * @brief Абсолютная
	 *
	 */
	abs,
	/**
	 * @brief Гринвическая
	 *
	 */
	grw,
	/**
	 * @brief Эклиптическая
	 *
	 */
	ecl
};
constexpr inline auto ort_cs = mathematical::ort;
constexpr inline auto sph_cs = mathematical::sph;
constexpr inline auto abs_cs = astronomical::abs;
constexpr inline auto grw_cs = astronomical::grw;
constexpr inline auto ecl_cs = astronomical::ecl;

template <astronomical, mathematical, astronomical, mathematical>
struct transform;

/**
 * @brief Преобразование между сферической и ортогональной АСК
 */
template <>
struct transform<abs_cs, sph_cs, abs_cs, ort_cs>
{
	/**
	 * @brief Преобразование из сферической в ортогональную АСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param out вектор (x, y, z)
	 */
	static void forward(double const in[3], double out[3]);
	/**
	 * @brief Преобразование из ортогональной в сферическую АСК
	 *
	 * @param in вектор (x, y, z)
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(double const in[3], double out[3]);
};
/**
 * @brief Преобразование между сферической и ортогональной ГСК
 */
template <>
struct transform<grw_cs, sph_cs, grw_cs, ort_cs>
{
	/**
	 * @brief Преобразование из сферической в ортогональную ГСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param out вектор (x, y, z)
	 */
	static void forward(double const in[3], double out[3]);
	/**
	 * @brief Преобразование из ортогональной в сферическую ГСК
	 *
	 * @param in вектор (x, y, z)
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void backward(double const in[3], double out[3]);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template <>
struct transform<abs_cs, ort_cs, grw_cs, ort_cs>
{
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор в АСК (x, y, z)
	 * @param t звёздное время
	 * @param out вектор в ГСК (x, y, z)
	 */
	static void forward(double const in[3], double t, double out[3]);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор в ГСК (x, y, z)
	 * @param t звёздное время
	 * @param out вектор в АСК (x, y, z)
	 */
	static void backward(double const in[3], double t, double out[3]);
	/**
	 * @brief Преобразование из АСК в ГСК.
	 *
	 * @param ar координаты в АСК (x, y, z)
	 * @param av скорости в АСК (vx, vy, vz)
	 * @param t звёздное время
	 * @param w угловая скорость вращения Земли (рад/с)
	 * @param gr координаты в ГСК (x, y, z)
	 * @param gv скорости в ГСК (vx, vy, vz)
	 */
	static void forward(double const ar[3], double const av[3], double t, double w, double gr[3], double gv[3]);
	/**
	 * @brief Преобразование из ГСК в АСК.
	 *
	 * @param gr координаты в ГСК (x, y, z)
	 * @param gv скорости в ГСК (vx, vy, vz)
	 * @param t звёздное время
	 * @param w угловая скорость вращения Земли (рад/с)
	 * @param ar координаты в АСК (x, y, z)
	 * @param av скорости в АСК (vx, vy, vz)
	 */
	static void backward(double const gr[3], double const gv[3], double t, double w, double ar[3], double av[3]);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template <>
struct transform<abs_cs, sph_cs, grw_cs, sph_cs>
{
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param t звёздное время
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double *const in, double t, double *const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param t звёздное время
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double *const in, double t, double *const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template <>
struct transform<abs_cs, sph_cs, grw_cs, ort_cs>
{
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (радиус, склонение, прямое восхождение)
	 * @param t звёздное время
	 * @param out вектор (x, y, z)
	 */
	static void forward(const double *const in, double t, double *const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (x, y, z)
	 * @param t звёздное время
	 * @param out вектор (радиус, склонение, прямое восхождение)
	 */
	static void backward(const double *const in, double t, double *const out);
};
/**
 * @brief Преобразование между АСК и ГСК
 */
template <>
struct transform<abs_cs, ort_cs, grw_cs, sph_cs>
{
	/**
	 * @brief Преобразование из АСК в ГСК
	 *
	 * @param in вектор (x, y, z)
	 * @param t звёздное время
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double *const in, double t, double *const out);
	/**
	 * @brief Преобразование из ГСК в АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param t звёздное время
	 * @param out вектор (x, y, z)
	 */
	static void backward(const double *const in, double t, double *const out);
};

/**
 * @brief Преобразование между АСК и эклиптической СК
 */
template <>
struct transform<abs_cs, ort_cs, ecl_cs, sph_cs>
{
	/**
	 * @brief Преобразование из АСК в эклиптическую СК
	 *
	 * @param in вектор (x, y, z)
	 * @param e наклон эклиптики к экватору
	 * @param out вектор (радиус, широта, долгота)
	 */
	static void forward(const double *const in, double e, double *const out);
	/**
	 * @brief Преобразование из эклиптической СК и АСК
	 *
	 * @param in вектор (радиус, широта, долгота)
	 * @param e наклон эклиптики к экватору
	 * @param out вектор (x, y, z)
	 */
	static void backward(const double *const in, double e, double *const out);
};
