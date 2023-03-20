#pragma once
#include <vector>

/**
 * @brief Коэф-ты гармоники потенциала
 *
 */
struct potential_harmonic
{
	double cos, sin;
};

// Простарнство имён для модели Земли EGM96
namespace egm
{
	/**
	 * @brief Гравитационная постоянная Земли
	 */
	constexpr inline double mu{0.3986004415E+15};
	/**
	 * @brief Радиус экватора в м
	 */
	constexpr inline double rad{0.6378136300E+07};
	/**
	 * @brief Квадрат эксцентриситета
	 */
	constexpr inline double ecsqr{6.69437999014e-3};
	/**
	 * @brief Угловая скорость вращения Земли вокруг оси в рад/с
	 */
	constexpr inline double angv{72.92115e-6};
	/**
	 * @brief Коэффициент полярного сжатия Земли
	 */
	constexpr inline double flat{1.0 / 298.257223563};
	/**
	 * @brief Количество гармоник геопотенциала в модели геопотенциала
	 */
	constexpr inline size_t count{360};
	/**
	 * @brief Массив грамоник геопотенциала
	 */
	extern std::vector<potential_harmonic> harmonics;

	/**
	 * @brief Копирование значений гармоник геопотенциала в массив.
	 *
	 * @param begin итератор на начало данных
	 * @param end итератор на конец данных
	 */
	template <typename iterator>
	void read_harmonics(iterator begin, iterator end)
	{
		egm::harmonics.resize((egm::count + 1) * (egm::count + 2) / 2);
		for (auto iter = egm::harmonics.begin(); begin != end; ++begin, ++iter)
		{
			*iter = *begin;
		}
	}
}

/**
 * @brief Тригонометрическая функция
 *
 */
struct trigonometric_func
{
	double cos, sin;
};

/**
 * @brief Гравитационный потенциал Земли
 *
 */
class geopotential
{
	/**
	 * @brief Гармоники синусов и косинусов долготы
	 */
	std::vector<trigonometric_func> _cs;
	/**
	 * @brief Значения полиномов Лежандра
	 */
	std::vector<double> _pnm;

private:
	void move(geopotential &other) noexcept;

public:
	geopotential();
	explicit geopotential(size_t count);
	geopotential(const geopotential &other) = default;
	geopotential(geopotential &&other) noexcept;
	geopotential &operator=(const geopotential &other) = default;
	geopotential &operator=(geopotential &&other) noexcept;

	/**
	 * @brief Вычисление значения потенциала
	 *
	 * @param v вектор в ГСК (x, y, z) [м]
	 * @return значение потенциала
	 */
	double operator()(const double *v);
	/**
	 * @brief Вычисление значения ускорения потенциала
	 *
	 * @param in вектор в ГСК (x, y, z) [м]
	 * @param out вектор (du/dx, du/dy, du/dz)
	 */
	void acceleration(const double in[3], double out[3]);
	/**
	 * @brief Вычисление вектора из производных потенциала и матрицы вторых производных по координатам.
	 *
	 * @param in вектор в ГСК (x, y, z) [м]
	 * @param outv вектор производных потенциала (du/dx, du/dy, du/dz)
	 * @param outm матрица вторых производных ((ddu/ddx, ddu/dxdy, ddu/dxdz), (ddu/dydx, ddu/ddy, ddudydz), (ddu/dzdx, ddu/dzdy, ddu/ddz))
	 */
	void acceleration(double const in[3], double outv[3], double outm[3][3]);
};

/**
 * @brief Параметры Солнца
 *
 */
struct solar_model
{
	/**
	 * @brief Вычисление координат
	 *
	 * @param t время с начала 1970 года (сек)
	 * @param ort вектор в АСК (x, y, z) в м
	 * @param sph вектор в АСК (радиус [м], склонение [рад], прямое восхождение [рад])
	 */
	static void coordinates(int64_t t, double *ort = nullptr, double *sph = nullptr);
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
struct lunar_model
{
	/**
	 * @brief Вычисление координат
	 *
	 * @param t время с начала 1970 года (сек)
	 * @param out вектор в АСК (x, y, z)
	 */
	static void coordinates(int64_t t, double *const out);
	/**
	 * @brief Гравитационная постоянная
	 */
	static constexpr inline double mu() { return 4.90486959e12; }
};

/**
 * @brief Вычисление ускорения в точке, вызванного притяжением массивного тела
 *
 * @param p точка (x, y, z) [м]
 * @param m координаты центра масс тела (x, y, z) [м]
 * @param mu гравитационная постоянная тела
 * @param a ускорение (x, y, z) [м/с]
 */
void mass_acceleration(const double *p, const double *m, double mu, double *a);

/**
 * @brief Звездное время
 *
 * @param t время с начала 1970 года (мс)
 * @return звёздное время в рад
 */
double sidereal_time(int64_t t);

/**
 * @brief Вычисление высоты над земным эллипсоидом
 *
 * @param v вектор в ГСК (x, y, z) [м]
 * @param r радиус экватора
 * @param f полярное сжатие
 * @return высота в м
 */
double height_above_ellipsoid(const double *v, double r, double f);
