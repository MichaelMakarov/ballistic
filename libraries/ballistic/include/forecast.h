#pragma once
#include <integrators.h>
#include <linalg.h>
#include <times.h>

#include <vector>
#include <algorithm>

/**
 * @brief Параметры движения размерности _dim
 */
template<size_t _dim >
struct params_of_motion : npair<vec<_dim>, time_h>
{
	/**
	 * @brief Номер витка
	 *
	 */
	size_t n{};
};

/**
 * @brief Параметры движения
 *
 */
using motion_params = params_of_motion<6>;

constexpr double defstep{ 30 };

/**
 * @brief Прогноз параметров движения
 *
 * @tparam dim размерность вектора состояния
 */
template<size_t dim>
struct forecast {
	/**
	 * @brief Список параметров движения
	 *
	 */
	std::vector<params_of_motion<dim>> _params;
	/**
	 * @brief Шаг интегрирования в сек
	 *
	 */
	double _step;

	forecast() : _step{ defstep } {}
	forecast(const forecast&) = default;
	forecast(forecast&& other) noexcept : _params{ std::move(other._params) }, _step{ other._step } {}

	forecast& operator=(const forecast&) = default;
	forecast& operator=(forecast&& other) noexcept;

	/**
	 * @brief Вычисление параметров движения на заданный момент времени
	 *
	 * @tparam degree степень аппроксимации
	 * @param t момент времени
	 * @return параметры движения
	 */
	template<size_t degree = 4>
	params_of_motion<dim> params(time_h t) const;
	/**
	 * @brief Расчёт параметров движения
	 *
	 * @tparam F тип функции
	 * @param mp начальные параметры движения
	 * @param tk конечный момент времени
	 * @param fn функция правой части диф. ур-ния движения
	 * @param step шаг интегрирования
	 */
	template<typename F>
	void run(const params_of_motion<dim>& mp, time_h tk, F&& fn, double step = defstep);

private:
	template<typename F>
	void start_run(size_t index, F&& fn);
	template<typename F>
	void continue_run(size_t index, F&& fn);


};

template<size_t dim>
inline forecast<dim>& forecast<dim>::operator=(forecast&& other) noexcept
{
	_params = std::move(other._params);
	_step = other._step;
	return *this;
}

template<size_t dim>
template<typename F>
inline void forecast<dim>::run(const params_of_motion<dim>& mp, time_h tk, F&& fn, double step)
{
	if (std::fabs(step) < std::numeric_limits<double>::epsilon() * 0.5) {
		throw std::invalid_argument("шаг интегрирования имеет значение близкое к нулю");
	}
	if (std::signbit(tk - mp.t) != std::signbit(step)) {
		throw std::invalid_argument("знак шага интегрирования не совпадает со знаком временного интервала");
	}

	auto count = static_cast<size_t>(std::ceil((tk - mp.t) / step)) + 1;
	_params.resize(count);
	_params[0] = mp;
	this->_step = step;
	constexpr auto index = abm8<vec<dim>, time_h>::degree - 1;

	if (index < count) {
		start_run(index, fn);
		continue_run(index, fn);
	} else {
		start_run(count, fn);
	}
}

template<size_t dim>
template<typename F>
inline void forecast<dim>::start_run(size_t index, F&& func)
{
	auto in = _params[0];
	decltype(in) out;
	bool intersection;
	size_t n{ 6 };
	double step{ this->_step / n };
	rk4<vec<dim>, time_h> integrator;
	for (size_t i{}; i < index; ++i) {
		for (size_t k{}; k < n; ++k) {
			integrator.integrate(in, step, out, func);
			intersection = std::signbit(in.v[2]) && !std::signbit(out.v[2]);
			in = out;
			if (intersection) ++out.n;
		}
		_params[i + 1] = out;
	}
}

template<size_t dim>
template<typename F>
inline void forecast<dim>::continue_run(size_t index, F&& func) {
	bool intersection;
	abm8<vec<dim>, time_h> integrator;
	auto it = std::begin(_params);
	for (size_t i{ index }; i < _params.size() - 1; ++i) {
		integrator.integrate(it, _step, _params[i + 1], func);
		intersection = std::signbit(_params[i].v[2]) && !std::signbit(_params[i + 1].v[2]);
		if (intersection) _params[i + 1].n = _params[i].n + 1;
		++it;
	}
}

template<size_t dim>
template<size_t degree>
inline params_of_motion<dim> forecast<dim>::params(time_h t) const
{
	auto count = _params.size();
	if (count < degree) {
		throw std::length_error("степерь аппроксимации первышает кол-во доступных точек");
	}
	auto index = static_cast<size_t>((t - _params[0].t) / _step);
	if (index >= count) {
		throw std::invalid_argument("момент времени находится за пределами прогноза");
	}
	// проверка знака координаты z для ближайшей по времени точки
	bool intersection = std::signbit(_params[index].v[2]);
	// номер витка
	size_t loop = _params[index].n;
	// индекс первой точки для аппроксимации
	index = std::max(int{}, std::min(int(count) - int(degree), int(index) - int(degree / 2)));
	// P(t) = sum{n = 0..dim} (mult{r = 0..dim, r != n} (t - t_m)/(t_n - t_m)) x_n
	vec<dim> res;
	double mult;
	for (size_t n{}; n < degree; ++n) {
		mult = 1;
		for (size_t k{}; k < degree; ++k) {
			if (k != n) {
				double up = t - _params[index + k].t;
				double down = _params[index + n].t - _params[index + k].t;
				mult *= (t - _params[index + k].t) / (_params[index + n].t - _params[index + k].t);
			}
		}
		res += mult * _params[index + n].v;
	}
	// если координата z полученной точки другого знака
	intersection &= !std::signbit(res[2]);
	if (intersection) ++loop;
	return params_of_motion<dim>{ res, t, loop };
}
