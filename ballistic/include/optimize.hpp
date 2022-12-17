#pragma once
#include <maths.hpp>

void throw_if_index_out_of_range(size_t i, size_t supr);

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
/**
 * @brief Сравнение значений с заданной точностью, а также проверка, что новое значение не превосходит старое.
 *
 * @param oldval старое значение
 * @param newval новое значение
 * @param eps точность сравнения
 * @return true значения эквивалентны или новое значение больше старого
 * @return false значения не эквивалентны
 */
bool is_equal_or_greater(double oldval, double newval, double eps);

namespace detail
{
	class invoker
	{
	public:
		virtual void invoke(size_t) const = 0;
	};

	template <typename F>
	class function_invoker : public invoker
	{
		F &_func;

	public:
		function_invoker(F &func) : _func{func} {}
		void invoke(size_t index) const override
		{
			_func(index);
		}
	};

}

/**
 * @brief Выполнение функции в параллельном режиме.
 *
 * @tparam F тип вызываемой функции
 * @param begin начальный индекс
 * @param end конечный индекс
 * @param func функция
 */
template <typename F>
void parallel_for(size_t begin, size_t end, F &&func)
{
	static_assert(std::is_invocable<F, size_t>::value, "Функция не может быть вызвана с аргументом типа size_t и не удовлеворяет интерфейсу parallel_for.");
	void parallel_for_impl(size_t, size_t, detail::invoker *);
	if (begin != end)
	{
		parallel_for_impl(begin, end, new detail::function_invoker<F>(func));
	}
}

/**
 * @brief Интерфейс для оптимизации
 *
 * @tparam param кол-во оптимизируемых параметров
 */
template <size_t param>
class optimization_interface
{
public:
	/**
	 * @brief Вычисление значения функции невязок.
	 *
	 * @param v вектор параметров
	 * @return double
	 */
	virtual double residual(vec<param> const &v) const = 0;
	/**
	 * @brief Возвращает вектор из вариаций параметров.
	 *
	 * @return vec<param>
	 */
	virtual vec<param> variations() const = 0;
	/**
	 * @brief Уточняет частную производную для параметра с указанным индексом.
	 *
	 * @param deriv значение частной производной для параметра
	 * @param index индекс параметра
	 * @return double
	 */
	virtual double derivative(double deriv, size_t index) const = 0;
};

/**
 * @brief Оптимизация методом градиентного спуска.
 *
 * @tparam param кол-во оптимизируемых параметров
 * @param params оптимизируемые параметры
 * @param interface интерфейс для вычисления невязок
 * @param eps точность вычисления
 * @param maxiter максимальное кол-во итераций
 * @return size_t кол-во выполненных итераций
 */
template <size_t param>
size_t gradient_descent(vec<param> &params, optimization_interface<param> &interface, double eps = 1e-3, size_t maxiter = 10)
{
	// вариации параметров
	auto vars = interface.variations();
	// невязка на предыдущем шаге спуска
	double ext_resid{};
	// номер итерации спуска
	size_t ext_iteration{1};
	// цикл спуска
	for (; ext_iteration <= maxiter; ++ext_iteration)
	{
		// градиент функции невязок
		vec<param> gradient;
		{
			// массив невязок
			double residuals[param + 1]{};
			// массив векторов с параметрами, где последний вектор неизменённый, а остальные с вариациями
			vec<param> var_params[param + 1]{};
			for (size_t i{}; i < param; ++i)
			{
				auto &ref = var_params[i] = params;
				ref[i] += vars[i];
			}
			var_params[param] = params;
			parallel_for(0, param + 1, [&var_params, &interface, &residuals](size_t i)
						 { residuals[i] = interface.residual(var_params[i]); });
			for (size_t i{}; i < param; ++i)
				gradient[i] = interface.derivative((residuals[i] - residuals[param]) / vars[i], i);
			ext_resid = residuals[param];
		}
		// множитель оптимизации функции F(x_n + lambda * gradient)
		double lambda{1};
		// невязка на предыдущем шаге уточнения множителя
		double int_resid = interface.residual(params - gradient * lambda);
		for (size_t int_iteration{1}; int_iteration <= maxiter; ++int_iteration)
		{
			constexpr double mult{0.1};
			auto cur_resid = interface.residual(params - gradient * (lambda * (mult + 1)));
			if (is_equal(int_resid, cur_resid, eps))
				break;
			lambda += (cur_resid - int_resid) / (mult * lambda);
			int_resid = cur_resid;
		}
		if (is_equal(ext_resid, int_resid, eps))
			break;
		params -= gradient * lambda;
	}
	return std::min(ext_iteration, maxiter);
}

template <size_t _size>
class jacoby_matrix
{
	optimization_interface<_size> const &_interface;
	vec<_size + 1> _dv;

public:
	explicit jacoby_matrix(optimization_interface<_size> const &interface) : _interface{interface}
	{
		auto dv = interface.variations();
		for (size_t i{}; i < _size; ++i)
			_dv[i] = dv[i];
	}
	std::pair<mat<_size, 1>, double> operator()(vec<_size> const &v) const
	{
		// массив невязок
		double residuals[_size + 1]{};
		// функция вычисления невязок по векторам с вариациями
		auto compute_func = [&v, this, &residuals](size_t i)
		{
			vec<_size> p{v};
			// добавляем вариацию
			p[i] += _dv[i];
			// рассчитываем значение функции невязок
			residuals[i] = _interface.residual(p);
		};
		parallel_for(0, _size + 1, compute_func);
		// матрица из частных производных
		mat<_size, 1> mx;
		for (size_t i{}; i < _size; ++i)
			mx[i][0] = _interface.derivative((residuals[i] - residuals[_size]) / _dv[i], i);
		return std::make_pair(mx, residuals[_size]);
	}
};

template <size_t _size>
struct temporary_params
{
	double m{5};
	double r{};
	mat<_size, 1> dv;
};

template <size_t _size>
class optimization_helper
{
	optimization_interface<_size> const &_interface;
	vec<_size> const &_v;
	mat<_size, 1> const &_jm;
	mat<_size, _size> _sm;

public:
	optimization_helper(optimization_interface<_size> const &interface, vec<_size> const v, mat<_size, 1> const jm) : _interface{interface}, _v{v}, _jm{jm}
	{
		_sm = _jm * transpose(_jm);
	}
	void operator()(temporary_params<_size> &p, double resid) const
	{
		double mult{p.m + 1};
		mat<_size, _size> sys_mx{_sm};
		for (size_t i{}; i < _size; ++i)
			sys_mx[i][i] *= mult;
		p.dv = inverse(sys_mx) * (_jm * (-resid));
		vec<_size> v{_v};
		for (size_t j{}; j < _size; ++j)
			v[j] += p.dv[j][0];
		p.r = _interface.residual(v);
	}
};

template <size_t _size>
auto optimize_multiplier(optimization_helper<_size> const &helper, temporary_params<_size> &res, double eps, size_t maxiter)
{
	// невязка на входе
	double resid{res.r};
	// i-1 and i
	temporary_params<_size> tmp_arr[2]{res};
	// индексы на i - 1 и i итерациях
	size_t prev{}, next{1};
	// сдвигаем на единицу вправо
	tmp_arr[next].m = tmp_arr[prev].m + 1;
	// вычисляем для i - 1 -го
	helper(tmp_arr[prev], resid);
	for (size_t iteration{1}; iteration <= maxiter; ++iteration)
	{
		// рассчитываем на i-ом шаге
		helper(tmp_arr[next], resid);
		//  условие, что вариация множителя вызывает ощутимое изменение невязки
		bool equal = is_equal_or_greater(tmp_arr[prev].r, tmp_arr[next].r, eps);
		if (tmp_arr[next].r < res.r)
		{
			// обновление
			res = tmp_arr[next];
		}
		// проверка условия останова
		if (equal)
			break;
		// коррекция множителя
		auto cor = tmp_arr[prev].r * (tmp_arr[next].m - tmp_arr[prev].m) / (tmp_arr[next].r - tmp_arr[prev].r);
		tmp_arr[prev].m -= cor;
		// меняем индексы местами
		std::swap(next, prev);
	}
	// // выстраиваем значения множителя в порядке возрастания
	// if (tmp_arr[next].m < tmp_arr[prev].m)
	// 	std::swap(next, prev);
	// // золотое сечение
	// constexpr auto golden_ratio_inv{1 / 1.618};
	// for (size_t iteration{1}; iteration < maxiter; ++iteration)
	// {
	// 	auto delta = (tmp_arr[next].m - tmp_arr[prev].m) * golden_ratio_inv;
	// 	auto next_m = tmp_arr[next].m;
	// 	auto prev_m = tmp_arr[prev].m;
	// 	tmp_arr[next].m = prev_m + delta;
	// 	tmp_arr[prev].m = next_m - delta;
	// 	parallel_for(0, 2, [&tmp_arr, &helper, resid](size_t i)
	// 				 { helper(tmp_arr[i], resid); });
	// 	// если значение невязки в левом значении больше, чем в правом, то сдвигаем левый конец
	// 	if (tmp_arr[prev].r > tmp_arr[next].r)
	// 		tmp_arr[next].m = next_m;
	// 	// иначе правый
	// 	else
	// 		tmp_arr[prev].m = prev_m;
	// 	bool equal = is_equal(tmp_arr[next].m, tmp_arr[prev].m, eps);
	// 	if (equal)
	// 	{
	// 		break;
	// 		res = tmp_arr[next];
	// 	}
	// }
}

/**
 * @brief Итерация оптимизации
 *
 * @tparam _size размерность вектора оптимизируемых параметров
 */
template <size_t _size>
struct optimization_iteration
{
	/**
	 * @brief Номер итерации
	 *
	 */
	size_t n;
	/**
	 * @brief Значение функции невязки
	 *
	 */
	double r;
	/**
	 * @brief Вектор оптимизируемых параметров
	 *
	 */
	vec<_size> v;
	/**
	 * @brief Вектор из частных производных
	 *
	 */
	vec<_size> m;
	/**
	 * @brief Вектор корректирующих значений
	 *
	 */
	vec<_size> dv;
};

template <size_t _size>
class optimization_logger
{
public:
	virtual void add(optimization_iteration<_size> const &) = 0;
};

template <size_t _size>
void levenberg_marquardt(vec<_size> &params, optimization_interface<_size> const &interface, optimization_logger<_size> *logger = nullptr, double eps = 1e-3, size_t maxiter = 20)
{
	// вычислитель матрицы Якоби
	jacoby_matrix<_size> jm(interface);
	// параметры оптимизации множителя
	temporary_params<_size> tmp;
	// цикл спуска
	for (size_t iteration{1}; iteration <= maxiter; ++iteration)
	{
		// матрица Якоби из частных производных и невязка
		auto [jacoby_mx, ext_resid] = jm(params);
		tmp.r = ext_resid;
		// оптимизация множителя
		optimize_multiplier(optimization_helper<_size>(interface, params, jacoby_mx), tmp, eps, maxiter);
		bool equal = is_equal(ext_resid, tmp.r, eps);
		if (logger)
		{
			optimization_iteration<_size> iter;
			iter.n = iteration;
			iter.v = params;
			iter.r = ext_resid;
			if (!equal)
			{
				std::copy(tmp.dv.data(), tmp.dv.data() + _size, iter.dv.data());
				std::copy(jacoby_mx.data(), jacoby_mx.data() + _size, iter.m.data());
			}
			logger->add(iter);
		}
		if (equal)
			return;
		for (size_t i{}; i < _size; ++i)
			params[i] += tmp.dv[i][0];
	}
}