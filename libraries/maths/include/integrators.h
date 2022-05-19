#pragma once

template<typename>
struct function_type {
	constexpr static bool is_function_ptr = false;
};

template<typename Res, typename ... Args>
struct function_type<Res(*)(Args...)> {
	constexpr static bool is_function_ptr = true;
};

template<typename> struct function_signature;

template<typename Res, typename ... Args>
struct function_signature<Res(*)(Args...)> {
	using signature_t = Res(*)(Args...);
};

template<typename> struct method_signature;

template<typename Obj, typename Res, typename ... Args>
struct method_signature<Res(Obj::*)(Args...)> : public function_signature<Res(*)(Args...)> {};

template<typename Obj, typename Res, typename ... Args>
struct method_signature<Res(Obj::*)(Args...) const> : public function_signature<Res(*)(Args...)> {};

template<bool, typename> struct function_ptr;

template<typename Func>
struct function_ptr<true, Func> : public function_signature<Func> {};

template<typename Func>
struct function_ptr<false, Func> : public method_signature<decltype(&Func::operator())> {};

template<typename Func>
struct callable_deducer : function_ptr<function_type<Func>::is_function_ptr, Func> {};


#include <type_traits>


/**
 * @brief Единица данных для интегрирования
 *
 * @tparam X тип вектора
 * @tparam T тип независимой переменной
 */
template<typename V, typename T>
struct npair {
	/**
	 * @brief вектор
	 */
	V v;
	/**
	 * @brief время
	 */
	T t;
};

template<typename X, typename T, typename F>
struct assignment_for_rk : public std::enable_if<
	std::is_same_v<X(*)(const X&, const T&), typename callable_deducer<F>::signature_t>, void>
{};

/**
 * @brief Явный интегратор Рунге-Кутты
 *
 * @tparam X тип вектор-функции
 * @tparam T тип переменной
 */
template<typename X, typename T>
class rk4 {
public:
	/**
	 * @brief Один шаг интегрирования.
	 *
	 * @tparam F тип функции правой части
	 * @param p0 начальное значение
	 * @param step шаг интегрирования
	 * @param pk выходное значение
	 * @param func ф-ция правой части диф. ур-ния
	 */
	template<typename F,
			 typename O = typename assignment_for_rk<X, T, std::remove_reference<F>::type>::type>
	void integrate(const npair<X, T>& p0, double step, npair<X, T>& pk, F&& func) const
	{
		double step_2 = 0.5 * step, step_6 = step / 6.0;
		auto t = p0.t + step_2;
		pk.t = p0.t + step;
		auto k1{ func(p0.v, p0.t) };
		auto k2{ func(p0.v + k1 * step_2, t) };
		auto k3{ func(p0.v + k2 * step_2, t) };
		auto k4{ func(p0.v + k3 * step,  pk.t) };
		pk.v = p0.v + (k1 + (k2 + k3) * 2.0 + k4) * step_6;
	}
};

#include <iterator>

template<typename X, typename T, typename F, typename I>
struct assignment_for_adams : public std::enable_if<
	std::is_base_of_v<npair<X, T>, typename std::iterator_traits<I>::value_type> &&
	std::is_same_v<X(*)(const X&, const T&), typename callable_deducer<F>::signature_t>, void>
{};
/**
 * @brief Многошаговый интегратор Адамса-Башфорта-Моултона
 *
 * @tparam X тип вектор-функции
 * @tparam T тип независимой переменной
 */
template<typename X, typename T>
class abm8 {
	const double _b[8]{
		-0.3042245370370370572,
		2.445163690476190421,
		-8.612127976190476986,
		17.37965443121693454,
		-22.02775297619047734,
		18.05453869047619264,
		-9.525206679894179018,
		3.589955357142857295
	};
	const double _c[8]{
		0.01136739417989418056,
		-0.09384093915343914849,
		0.343080357142857173,
		-0.732035383597883671,
		1.017964616402116551,
		-1.0069196428571429713,
		1.156159060846560838,
		0.3042245370370370017
	};
public:
	/**
	 * @brief Степень интегратора
	 */
	constexpr const static size_t degree{ 8 };
	/**
	 * @brief Один шаг интегрирования.
	 *
	 * @tparam F тип функции
	 * @param it итератор на первый элемент
	 * @param step шаг интегрирования
	 * @param pk выходное значение
	 * @param func функция правой части диф. ур-ния
	 */
	template<
		typename I, typename F,
		typename O = typename assignment_for_adams<X, T, std::remove_reference<F>::type, I>::type>
	void integrate(I it, double step, npair<X, T>& pk, F&& func) const
	{
		pk.v = X{};
		auto xt{ func(it->v, it->t) * _b[0] };
		X xb;
		for (size_t i{ 1 }; i < 8; ++i) {
			++it;
			xb = func(it->v, it->t);
			xt += xb * _b[i];		// предиктор
			pk.v += xb * _c[i - 1];	// корректор
		}
		xt *= step;
		xt += it->v;
		pk.t = it->t + step;
		pk.v += func(xt, pk.t) * _c[7];
		pk.v *= step;
		pk.v += it->v;
	}
};
