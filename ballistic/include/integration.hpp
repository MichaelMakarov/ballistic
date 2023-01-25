#pragma once
#include <formatting.hpp>
#include <vector>

namespace math
{

	inline auto min_of(std::size_t left, std::size_t right)
	{
		return left < right ? left : right;
	}

	/**
	 * @brief Интегратор
	 *
	 * @tparam V должен быть default constructible и иметь операторы *=(double), +=(V), *(double), +(V)
	 * @tparam T должен быть default constructible и иметь операторы -(T), -=(T)
	 * @tparam D тип величины промежутка интегрирования
	 */
	template <typename V, typename T, typename D>
	class integrator
	{
	public:
		struct pair
		{
			V v;
			T t;
		};

		std::vector<pair> _points;
		D _step;

		constexpr static std::size_t degree{8};
		constexpr static std::size_t ratio{6};
		constexpr static D zero{};

	public:
		integrator(integrator const &) = default;
		integrator(integrator &&other) noexcept : _points{std::move(other._points)}, _step{other._step} {}
		integrator &operator=(integrator const &) = default;
		integrator &operator=(integrator &&other) noexcept
		{
			_points.swap(other._points);
			_step = other._step;
			return *this;
		}
		/**
		 * @brief Construct a new integrator object
		 *
		 * @tparam F тип функции правой части с сигнатурой V(*)(V const &, T const &)
		 * @param point исходная точка
		 * @param tk конечное значение промежутка интегрирования
		 * @param func функция праввой части
		 * @param step шаг интегрирования
		 */
		template <typename F>
		integrator(V const &v, T const &tn, T const &tk, F &&func, D const &step)
		{
			if (step == zero)
				throw_invalid_argument("Шаг интегрирования должен быть отличен от нуля.");
			if ((step > zero) != (tk > tn))
				throw_invalid_argument("Знак щага интегрирования не соответствует знаку промежутка интегрирования.");
			auto count = static_cast<std::size_t>((tk - tn) / step) + 1;
			_points.resize(count);
			_points.front() = {v, tn};
			_step = step;
			std::size_t index = count > degree ? degree : count;
			// массив производных (значений правой части)
			V arr[degree]{};
			// шаг разгона
			D delta = step / ratio;
			// разгон
			for (std::size_t i{1}; i < index; ++i)
			{
				pair tmp = _points[i - 1];
				arr[i - 1] = func(tmp.v, tmp.t);
				for (std::size_t k{}; k < ratio; ++k)
				{
					tmp = rk4(tmp, delta, func);
					_points[i] = tmp;
				}
			}
			if (index == degree)
				arr[degree - 1] = func(_points[7].v, _points[7].t);
			// основной цикл
			for (std::size_t i{index}; i < count; ++i)
			{
				_points[i] = adams(arr, _points[i - 1], step, func);
			}
		}
		/**
		 * @brief Возвращает вектор на заданный момент.
		 *
		 * @tparam degree используемое кол-во точек аппроксимации
		 * @param t момент
		 * @return integratable<V, T>
		 */
		template <std::size_t degree = 4>
		V point(T const &t) const
		{
			static_assert(degree > 1, "Недостаточное кол-во точек для аппроксимации.");
			std::size_t count = _points.size();
			T tn = _points.front().t;
			T tk = _points.back().t;
			if (count < degree)
				throw_length_error("Степень аппроксимации превосходит кол-во доступных точек.");
			// индекс первой точки для аппроксимации
			auto index = static_cast<std::size_t>((t - tn) / _step);
			if (index > count)
				throw_invalid_argument("Момент времени % находится за пределами интервала интегрирования % - %.", t, tn, tk);
			index -= min_of(index, degree / 2);
			index = min_of(index, count - degree);
			// P(t) = sum{n = 0..dim} (mult{k = 0..dim, k != n} (t - t_k)/(t_n - t_k)) x_n
			V r;
			D mult;
			for (size_t n{}; n < degree; ++n)
			{
				mult = 1;
				for (size_t k{}; k < degree; ++k)
				{
					if (k != n)
					{
						D up = t - _points[index + k].t;
						D down = _points[index + n].t - _points[index + k].t;
						mult *= static_cast<double>(up) / static_cast<double>(down);
					}
				}
				r += mult * _points[index + n].v;
			}
			return r;
		}

	private:
		template <typename F>
		pair rk4(pair const &in, D const &step, F &func)
		{
			pair out{};
			D step_2 = step / 2, step_6 = step / 6;
			T t = in.t + step_2;
			out.t = in.t + step;
			V k1 = func(in.v, in.t);
			V k2 = func(in.v + k1 * step_2, t);
			V k3 = func(in.v + k2 * step_2, t);
			V k4 = func(in.v + k3 * step, out.t);
			out.v = in.v + (k1 + (k2 + k3) * 2.0 + k4) * step_6;
			return out;
		}

		template <typename F>
		pair adams(V (&arr)[8], pair const &in, D const &step, F &func)
		{
			constexpr double b[8]{
				-0.3042245370370370572, 2.445163690476190421, -8.612127976190476986, 17.37965443121693454,
				-22.02775297619047734, 18.05453869047619264, -9.525206679894179018, 3.589955357142857295};
			constexpr double c[8]{
				0.01136739417989418056, -0.09384093915343914849, 0.343080357142857173, -0.732035383597883671,
				1.017964616402116551, -1.0069196428571429713, 1.156159060846560838, 0.3042245370370370017};
			arr[7] = func(in.v, in.t);
			// значение по корректору
			pair out{};
			// значение по предиктору
			V x = arr[0] * b[0];
			for (std::size_t i{1}; i < 8; ++i)
			{
				// prediction
				x += arr[i] * b[i];
				// correction
				out.v += arr[i] * c[i - 1];
				// renew
				arr[i - 1] = arr[i];
			}
			x *= step;
			x += in.v;
			out.t = in.t + step;
			out.v += func(x, out.t) * c[7];
			out.v *= step;
			out.v += in.v;
			return out;
		}
	};

}