#pragma once
#include <maths.hpp>
#include <parallel.hpp>

namespace math
{

	/**
	 * @brief Сравнение значений с заданной относительной точностью.
	 *
	 * @param oldval старое значение
	 * @param newval новое значение
	 * @param eps точность сравнения
	 * @return true значения эквивалентны
	 * @return false значения не эквивалентны
	 */
	bool is_equal(double oldval, double newval, double eps);

	template <std::size_t _dim>
	using array_view = double[_dim];

	template <std::size_t _size>
	struct residual_function
	{
		static double residual(vector const &v)
		{
			return v * v;
		}
	};

	/**
	 * @brief Итерация оптимизации
	 *
	 * @tparam _size размерность вектора оптимизируемых параметров
	 */
	template <std::size_t _size>
	struct optimization_iteration
	{
		/**
		 * @brief Номер итерации
		 *
		 */
		size_t n{};
		/**
		 * @brief Значение функции невязки
		 *
		 */
		double r{};
		/**
		 * @brief Вектор оптимизируемых параметров
		 *
		 */
		vec<_size> v;
		/**
		 * @brief Вектор невязок
		 *
		 */
		vector rv;
		/**
		 * @brief Матрица из частных производных
		 *
		 */
		matrix dm;
		/**
		 * @brief Вектор корректирующих значений
		 *
		 */
		vector dv;

		optimization_iteration() = default;
		optimization_iteration(optimization_iteration const &) = default;
		optimization_iteration(optimization_iteration &&other) noexcept : n{other.n}, r{other.r}, v{other.v}, rv{std::move(other.rv)}, dm{std::move(other.dm)}, dv{std::move(other.dv)} {}
		optimization_iteration &operator=(optimization_iteration const &) = default;
		optimization_iteration &operator=(optimization_iteration &&other) noexcept
		{
			n = other.n;
			r = other.r;
			v = other.v;
			rv = std::move(other.rv);
			dm = std::move(other.dm);
			dv = std::move(other.dv);
			return *this;
		}
	};

	template <std::size_t _size>
	class optimization_logger
	{
	public:
		virtual void add(optimization_iteration<_size> &&) = 0;
	};

	/**
	 * @brief Интерфейс для оптимизации
	 *
	 * @tparam _size кол-во оптимизируемых параметров
	 * @tparam _dim размер вектора невязок, соответствующего одному измерению
	 */
	template <std::size_t _size, std::size_t _dim>
	class optimization_interface
	{
	public:
		/**
		 * @brief Кол-во измерений.
		 *
		 * @return std::size_t
		 */
		virtual std::size_t points_count() const = 0;
		/**
		 * @brief Вычисление невязок.
		 *
		 * @param v вектор параметров
		 * @param r массив невязок
		 */
		virtual void residual(vec<_size> const &v, array_view<_dim> *r) const = 0;
		/**
		 * @brief Возвращает вектор из вариаций параметров.
		 *
		 * @return vec<_size>
		 */
		virtual vec<_size> variations() const = 0;
		/**
		 * @brief Уточняет поправку для параметра с указанным индексом.
		 *
		 * @param value значение
		 * @param index индекс параметра
		 * @param add значение поправки
		 */
		virtual void update(double &value, std::size_t index, double add) const = 0;
	};

	template <size_t _size, size_t _dim>
	class equation_maker
	{
		optimization_interface<_size, _dim> const &_interface;
		double _dv[_size + 1]{};

	public:
		equation_maker(optimization_interface<_size, _dim> const &interface) : _interface{interface}
		{
			auto dv = interface.variations();
			for (size_t i{}; i < _size; ++i)
				_dv[i] = dv[i];
		}
		void operator()(vec<_size> const &v, matrix &dm, vector &rv) const
		{
			// указатели на начало массивов, куда будут записываться невязки (строки матрицы из частных производных и вектор невязок)
			array_view<_dim> *ptrs[_size + 1];
			for (std::size_t i{}; i < _size; ++i)
				ptrs[i] = reinterpret_cast<array_view<_dim> *>(dm[i]);
			ptrs[_size] = reinterpret_cast<array_view<_dim> *>(rv.data());
			// функция вычисления невязок по векторам с вариациями
			auto compute_func = [&v, &ptrs, this](std::size_t i)
			{
				auto j = i % _size;
				vec<_size> p{v};
				// добавляем вариацию
				p[j] += _dv[i];
				// заполняем вектор невязок
				_interface.residual(p, ptrs[i]);
			};
			par::parallel_for(size_t{}, _size + 1, compute_func, _size + 1);
			// формируем матрицу из частных производных
			for (std::size_t r{}; r < dm.rows(); ++r)
			{
				for (std::size_t c{}; c < dm.columns(); ++c)
				{
					double &val = dm[r][c];
					val = rv[c] - val;
					val /= _dv[r];
				}
			}
		}
	};

	struct optimization_info
	{
		/**
		 * @brief Множитель в матрице из алгоритма Левенберга-Марквардта
		 *
		 */
		double m{};
		/**
		 * @brief Значение функции невязок
		 *
		 */
		double r{};
		/**
		 * @brief Корректирующие значения параметров
		 *
		 */
		vector dv;
	};

	template <std::size_t _size, std::size_t _dim>
	class optimization_helper
	{
		optimization_interface<_size, _dim> const &_interface;
		vec<_size> const &_v;
		vector _rv;
		matrix _sm;

	public:
		optimization_helper(optimization_interface<_size, _dim> const &interface, vec<_size> const &v, matrix const &jm, vector const &rv) : _interface{interface}, _v{v}
		{
			_sm = jm * transpose(jm);
			_rv = jm * rv;
		}
		void operator()(optimization_info &p) const
		{
			double mult{p.m + 1};
			matrix sm{_sm};
			for (size_t i{}; i < _size; ++i)
				sm[i][i] *= mult;
			inverse(sm);
			p.dv = sm * _rv;
			vec<_size> v{_v};
			for (size_t j{}; j < _size; ++j)
				v[j] += p.dv[j];
			vector rv(_interface.points_count() * _dim);
			_interface.residual(v, reinterpret_cast<array_view<_dim> *>(rv.data()));
			p.r = residual_function<_size>::residual(rv);
		}
	};

	template <std::size_t _size, std::size_t _dim>
	void optimize_multiplier(optimization_helper<_size, _dim> const &helper, optimization_info &res, double eps, size_t maxiter)
	{
		double mul = 0.1;
		for (size_t iteration{1}; iteration <= maxiter; ++iteration)
		{
			optimization_info info_arr[3]{};
			info_arr[0].m = 0.5 * mul;
			info_arr[1].m = mul;
			info_arr[2].m = 1.5 * mul;
			par::parallel_for(info_arr, info_arr + 3, helper);
			//  условие, что вариация множителя вызывает ощутимое изменение невязки
			bool stop = is_equal(info_arr[0].r, info_arr[2].r, eps);
			stop |= info_arr[0].r > info_arr[1].r && info_arr[2].r > info_arr[1].r;
			// обновление
			// if (info_arr[1].r < res.r)
			res = info_arr[1];
			// проверка условия останова
			if (stop)
				break;
			// коррекция множителя
			auto cor = (info_arr[0].m - info_arr[2].m) / (info_arr[0].r - info_arr[2].r);
			if (cor > 0)
				mul *= 0.5;
			else
				mul *= 1.5;
		}
	}

	/**
	 * @brief Оптимизация параметров методом Левенберга-Марквардта.
	 *
	 * @tparam _size кол-во параметров
	 * @tparam _dim кол-во невязок, соотвествующих одному измерению
	 * @param v вектор оптимизируемых параметров
	 * @param interface интерфейс измерений
	 * @param logger логировщик
	 * @param eps точность вычислений
	 * @param maxiter максимальное кол-во итераций
	 */
	template <std::size_t _size, std::size_t _dim>
	void levmarq(vec<_size> &v, optimization_interface<_size, _dim> const &interface, optimization_logger<_size> *logger = nullptr, double eps = 1e-3, size_t maxiter = 20)
	{
		equation_maker<_size, _dim> eqm{interface};
		vector rv(interface.points_count() * _dim);
		matrix dm(_size, rv.size());
		// параметры оптимизации множителя
		optimization_info p;
		// цикл спуска
		for (size_t iteration{1}; iteration <= maxiter; ++iteration)
		{
			eqm(v, dm, rv); // вычисление матрицы частных производных и вектора невязок
			double res = p.r = residual_function<_size>::residual(rv);
			// оптимизация множителя
			optimize_multiplier(optimization_helper<_size, _dim>(interface, v, dm, rv), p, eps, maxiter);
			bool equal = is_equal(res, p.r, eps);
			if (logger)
			{
				optimization_iteration<_size> iter;
				iter.n = iteration;
				iter.v = v;
				iter.r = res;
				iter.dv = p.dv;
				iter.rv = rv;
				iter.dm = dm;
				logger->add(std::move(iter));
			}
			if (equal)
				return;
			for (size_t i{}; i < _size; ++i)
				v[i] += p.dv[i];
		}
	}

}