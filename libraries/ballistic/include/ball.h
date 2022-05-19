#pragma once
#include <vector>
#include <istream>

#include <times.h>


/**
 * @brief Коэф-ты гармоники потенциала
 *
 */
struct potential_harmonic { double cos, sin; };

enum struct egm_type : unsigned {
	JGM3, EGM96
};

template<egm_type>
struct egm_constants {
	constexpr static double mu{};
	constexpr static double rad{};
	constexpr static double ecsqr{};
	constexpr static double angv{};
	constexpr static double flat{};
	constexpr static size_t count{};
	static std::vector<potential_harmonic> harmonics;
};

template<egm_type type>
void load_harmonics_for(std::istream& instr) {
	using egmc_t = egm_constants<type>;
	egmc_t::harmonics.resize((egmc_t::count + 1) * (egmc_t::count + 2) / 2);
	std::copy(std::istream_iterator<potential_harmonic>{ instr }, {}, egmc_t::harmonics.begin());
}
template<egm_type type>
void load_harmonics_for(std::istream&& instr) {
	load_harmonics_for<type>(instr);
}

std::ostream& operator<<(std::ostream& , const potential_harmonic& );
std::istream& operator>> (std::istream& , potential_harmonic& );

template<>
struct egm_constants<egm_type::JGM3> {
	constexpr static double mu{ 0.3986004415E+15 };
	constexpr static double rad{ 0.6378136300E+07 };
	constexpr static double ecsqr{ 6.69437999014e-3 };
	constexpr static double angv{ 72.92115e-6 };
	constexpr static double flat{ 1.0 / 298.257223563 };
	constexpr static size_t count{ 70 };
	static std::vector<potential_harmonic> harmonics;

	static void load_harmonics(std::istream& instr) {
		load_harmonics_for<egm_type::JGM3>(instr);
	}
	void load_harmonics(std::istream&& instr) {
		load_harmonics_for<egm_type::JGM3>(std::forward<std::istream>(instr));
	}
};

template<>
struct egm_constants<egm_type::EGM96> {
	constexpr static double mu{ 0.3986004415E+15 };
	constexpr static double rad{ 0.6378136300E+07 };
	constexpr static double ecsqr{ 6.69437999014e-3 };
	constexpr static double angv{ 72.92115e-6 };
	constexpr static double flat{ 1.0 / 298.257223563 };
	constexpr static size_t count{ 360 };
	static std::vector<potential_harmonic> harmonics;

	static void load_harmonics(std::istream& instr) {
		load_harmonics_for<egm_type::EGM96>(instr);
	}
	static void load_harmonics(std::istream&& instr) {
		load_harmonics_for<egm_type::EGM96>(std::forward<std::istream>(instr));
	}
};

using JGM3 = egm_constants<egm_type::JGM3>;
using EGM96 = egm_constants<egm_type::EGM96>;

/**
 * @brief Тригонометрическая функция
 *
 */
struct trigonometric_func { double cos, sin; };

/**
 * @brief Гравитационный потенциал Земли
 *
 */
class geopotential {
	std::vector<trigonometric_func> _cs;
	std::vector<double> _pnm;
	const potential_harmonic* _harmonics{};
	size_t _count{};
	double _rad{}, _mu{};

private:
	void calc_trigonometric(double coslambda, double sinlambda);
	void calc_polynoms(double cosphi, double sinphi);

	void move(geopotential& other) noexcept;
public:
	geopotential() = default;
	template<egm_type type>
	geopotential(egm_constants<type>, size_t count);
	geopotential(const geopotential& other) = default;
	geopotential(geopotential&& other) noexcept;
	~geopotential() noexcept = default;

	geopotential& operator= (const geopotential& other) = default;
	geopotential& operator= (geopotential&& other) noexcept;

	/**
	 * @brief Вычисление значения потенциала
	 *
	 * @param v вектор в ГСК (x, y, z) [м]
	 * @return значение потенциала
	 */
	double operator() (const double* const v);
	/**
	 * @brief Вычисление значения ускорения потенциала
	 *
	 * @param in вектор в ГСК (x, y, z) [м]
	 * @param out вектор (dU/dx, dU/dy, dU/dz)
	 */
	void acceleration(const double* const in, double* const out);
};

template<egm_type type>
inline geopotential::geopotential(egm_constants<type>, size_t count) {
	_harmonics = egm_constants<type>::harmonics.data();
	_count = std::min(count, egm_constants<type>::count);
	_rad = egm_constants<type>::rad;
	_mu = egm_constants<type>::mu;
	size_t dim = ((_count + 1) * (_count + 2)) / 2;
	_cs.resize(_count + 1);
	_pnm.resize(dim + 2);
	_pnm[dim] = _pnm[dim + 1] = 0.0;
}

/**
 * @brief Параметры Солнца
 *
 */
struct solar_model {
	/**
	 * @brief Вычисление координат
	 *
	 * @param t время
	 * @param out вектор (радиус [м], склонение [рад], прямое восхождение [рад])
	 */

	/**
	 * @brief Вычисление координат
	 * 
	 * @param t время
	 * @param ort вектор в АСК (x, y, z) в м
	 * @param sph вектор в АСК (радиус [м], склонение [рад], прямое восхождение [рад])
	 */
	static void coordinates(time_h t, double* const ort = nullptr, double* const sph = nullptr);
	/**
	 * @brief Гравитационная постоянная
	 */
	static constexpr inline double mu() { return 1.327124400189e20; }
	/**
	 * @brief Радиус экватора [м]
	 */
	static constexpr inline double rad() { return 696340e3; }
	/**
	 * @brief Среднее расстояние до Земли [м]
	 */
	static constexpr inline double AU() { return 149597870700.0; }
	/**
	 * @brief Средняя величина светового потока
	 */
	static constexpr inline double flux() { return 1367.0; }
	/**
	 * @brief Средняя величина солнечного давления вблизи Земли (N * r^-2)
	 */
	static constexpr inline double pressure() { return 4.56e-6; }
};

/**
 * @brief Параметры Луны
 *
 */
struct lunar_model {
/**
 * @brief Вычисление координат
 *
 * @param t время
 * @param out вектор (радиус [м], склонение [рад], прямое восхождение [рад])
 */
static void coordinates(time_h t, double* const out);
/**
 * @brief Гравитационная постоянная
 */
static constexpr inline double mu() { return 4.90486959e12; }
};



/**
 * @brief Вычисление координат Солнца в ГСК
 *
 * @param t время
 * @param out вектор (x, y, z) [м]
 */
void solar_coordinates(time_h t, double* const out);
/**
 * @brief Вычисление координат Луны в ГСК
 *
 * @param t время
 * @param out вектор (x, y, z) [м]
 */
void lunar_coordinates(time_h t, double* const out);

/**
 * @brief Вычисление ускорения в точке, вызванного притяжением массивного тела
 *
 * @param p точка (x, y, z) [м]
 * @param m координаты центра масс тела (x, y, z) [м]
 * @param mu гравитационная постоянная тела
 * @param a ускорение (x, y, z) [м/с]
 */
void mass_acceleration(const double* const p, const double* const m, double mu, double* const a);


/**
 * @brief Среднее звёздное время
 *
 * @param t время
 * @return звёздное время в рад
 */
double sidereal_time_mean(time_h t);
/**
 * @brief Истинное звездное время
 *
 * @param t время
 * @return звёздное время в рад
 */
double sidereal_time_true(time_h t);

/**
 * @brief Вычисление высоты над земным эллипсоидом
 *
 * @param v вектор в ГСК (x, y, z) [м]
 * @param r радиус экватора
 * @param f полярное сжатие
 * @return высота в м
 */
double height_above_ellipsoid(const double* const v, double r, double f);
