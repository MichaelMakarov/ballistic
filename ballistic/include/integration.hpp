#pragma once
#include <formatting.hpp>
#include <vector>

template <typename>
struct is_function_ptr
{
	constexpr static bool value = false;
};

template <typename R, typename... Args>
struct is_function_ptr<R (*)(Args...)>
{
	constexpr static bool value = true;
};

template <typename>
struct signature;

template <typename R, typename... Args>
struct signature<R (*)(Args...)>
{
	using type = R (*)(Args...);
};

template <typename R, typename O, typename... Args>
struct signature<R (O::*)(Args...)> : signature<R (*)(Args...)>
{
};

template <typename R, typename O, typename... Args>
struct signature<R (O::*)(Args...) const> : signature<R (*)(Args...)>
{
};

template <bool, typename>
struct functor;

template <typename F>
struct functor<true, F> : signature<F>
{
};

template <typename F>
struct functor<false, F> : signature<decltype(&F::operator())>
{
};

template <typename F>
struct callable : functor<is_function_ptr<F>::value, std::remove_reference_t<F>>
{
};

/**
 * @brief Структура для интегрирования
 *
 * @tparam V вектор значение (поддерживает ариметические операции)
 * @tparam T независимая переменная (время)
 */
template <typename V, typename T>
struct integratable_point
{
	/**
	 * @brief Вектор
	 *
	 */
	V v;
	/**
	 * @brief Время
	 *
	 */
	T t;
};

template <typename V, typename T, typename F>
struct rk_check : std::enable_if<std::is_same_v<V (*)(V const &, T const &), typename callable<F>::type>, void>
{
};

/**
 * @brief Интегрирование методом Рунге-Кутта 4-го порядка.
 *
 * @tparam V тип вектора
 * @tparam T тип времени
 * @tparam D тип шага интегрирования
 * @tparam F тип функции правой части
 * @param in начальная точка
 * @param step шаг интегрирования
 * @param func ф-ция правой части
 * @return integratable_point<V, T>
 */
template <typename V, typename T, typename D, typename F, typename O = typename rk_check<V, T, F>::type>
auto rk4(integratable_point<V, T> const &in, D const &step, F &&func) -> integratable_point<V, T>
{
	integratable_point<V, T> out{};
	auto step_2 = step / 2, step_6 = step / 6;
	auto t = in.t + step_2;
	out.t = in.t + step;
	auto k1{func(in.v, in.t)};
	auto k2{func(in.v + k1 * step_2, t)};
	auto k3{func(in.v + k2 * step_2, t)};
	auto k4{func(in.v + k3 * step, out.t)};
	out.v = in.v + (k1 + (k2 + k3) * 2.0 + k4) * step_6;
	return out;
}

template <typename V, typename T, typename F, typename I>
struct adams_check : std::enable_if<
						 std::is_base_of_v<integratable_point<V, T>, typename std::iterator_traits<I>::value_type> &&
							 std::is_same_v<V (*)(V const &, T const &), typename callable<F>::type>,
						 void>
{
};

template <typename V, typename T, typename D, typename F, typename I, typename O = typename adams_check<V, T, F, I>::type>
auto adams(I it, D const &step, F &&func) -> integratable_point<V, T>
{
	constexpr double b[8]{
		-0.3042245370370370572, 2.445163690476190421, -8.612127976190476986, 17.37965443121693454,
		-22.02775297619047734, 18.05453869047619264, -9.525206679894179018, 3.589955357142857295};
	constexpr double c[8]{
		0.01136739417989418056, -0.09384093915343914849, 0.343080357142857173, -0.732035383597883671,
		1.017964616402116551, -1.0069196428571429713, 1.156159060846560838, 0.3042245370370370017};
	integratable_point<V, T> out{};
	V x{func(it->v, it->t) * b[0]};
	for (size_t i{1}; i < 8; ++i)
	{
		++it;
		V xb = func(it->v, it->t);
		x += xb * b[i];			// предиктор
		out.v += xb * c[i - 1]; // корректор
	}
	x = x * step;
	x += it->v;
	out.t = it->t + step;
	out.v += func(x, out.t) * c[7];
	out.v = out.v * step;
	out.v += it->v;
	return out;
}

/**
 * @brief Интерфейс интегрирования движения
 *
 * @tparam V тип вектора
 * @tparam T тип независимой переменной
 * @tparam D тип шага интегрирования
 */
template <typename V, typename T, typename D>
class integration_interface
{
public:
	/**
	 * @brief Массив точек параметров
	 *
	 */
	std::vector<integratable_point<V, T>> _points;
	/**
	 * @brief Используемый шаг интегрирования
	 *
	 */
	D _step;

	integration_interface() : _step{} {}
	integration_interface(integration_interface const &) = default;
	integration_interface(integration_interface &&) noexcept;
	/**
	 * @brief Запуск расчёта и заполнение данных.
	 *
	 * @tparam F тип функции правой части диффура
	 * @param point начальная точка
	 * @param tk конечное значение параметра интегрирования
	 * @param func функция правой части
	 * @param step шаг интегрирования
	 */
	template <typename F>
	integration_interface(integratable_point<V, T> const &point, T const &tk, F &&func, D const &step);

	integration_interface &operator=(integration_interface const &) = default;
	integration_interface &operator=(integration_interface &&) noexcept;

	/**
	 * @brief Расчёт точки на заданное значение параметра.
	 * Throws length_error when data is empty.
	 * Throws invalid_argument when invalid input.
	 *
	 * @tparam degree степень аппроксимации
	 * @param t время
	 * @return integratable_point<V, T>
	 */
	template <size_t degree = 4>
	integratable_point<V, T> point(T const &t) const;
};

template <typename V, typename T, typename D>
inline integration_interface<V, T, D>::integration_interface(integration_interface<V, T, D> &&other) noexcept
{
	_points = std::move(other._points);
	_step = other._step;
}

template <typename V, typename T, typename D>
inline integration_interface<V, T, D> &integration_interface<V, T, D>::operator=(integration_interface<V, T, D> &&other) noexcept
{
	_points = std::move(other._points);
	_step = other._step;
	return *this;
}

template <typename V, typename T, typename D>
template <typename F>
inline integration_interface<V, T, D>::integration_interface(integratable_point<V, T> const &point, T const &tk, F &&func, D const &step)
{
	constexpr D zero{};
	if (step == zero)
	{
		throw std::invalid_argument("Шаг интегрирования должен быть отличен от нуля.");
	}
	if ((step > zero) != (tk > point.t))
	{
		throw std::invalid_argument("Знак шага интегрирования не соответствует промежутку интегрирования.");
	}
	auto count = static_cast<size_t>((tk - point.t) / step) + 1;
	_points.resize(count);
	_points.front() = point;
	_step = step;
	constexpr size_t req{8};
	size_t index = std::min(req, count);

	integratable_point<V, T> in{point}, out{};
	constexpr size_t n{6};
	auto delta = step / n;
	// разгон
	for (size_t i{1}; i < index; ++i)
	{
		for (size_t k{}; k < n; ++k)
		{
			out = rk4(in, delta, func);
			in = out;
		}
		_points[i] = out;
	}

	// основной цикл расчёта
	for (auto it = std::begin(_points); index < count; ++it, ++index)
	{
		_points[index] = adams<V, T>(it, step, func);
	}
}

template <typename V, typename T, typename D>
template <size_t degree>
inline integratable_point<V, T> integration_interface<V, T, D>::point(T const &t) const
{
	auto count = _points.size();
	if (count < degree)
	{
		throw std::length_error(format("Степень аппроксимации % превосходит кол-во доступных точек %.", degree, count));
	}
	T tn = _points.front().t;
	T tk = _points.back().t;
	auto index = static_cast<size_t>((t - tn) / _step);
	if (index >= count)
	{
		throw std::invalid_argument(format("Момент времени % находится за пределами интервала интегрирования % - %.", t, tn, tk));
	}
	// индекс первой точки для аппроксимации
	index = (size_t)std::max(int{}, std::min(int(count) - int(degree), int(index) - int(degree / 2)));
	// P(t) = sum{n = 0..dim} (mult{r = 0..dim, r != n} (t - t_m)/(t_n - t_m)) x_n
	V r;
	D mult;
	for (size_t n{}; n < degree; ++n)
	{
		mult = 1;
		for (size_t k{}; k < degree; ++k)
		{
			if (k != n)
			{
				auto up = static_cast<D>(t - _points[index + k].t);
				auto down = static_cast<D>(_points[index + n].t - _points[index + k].t);
				mult *= up;
				mult /= down;
			}
		}
		r += mult * _points[index + n].v;
	}
	return {r, t};
}
