#pragma once
#include <integration.h>
#include <type_traits>


namespace math {
	template<typename X, typename T, typename F, typename Ix, typename It>
	struct assignment_for_adams : public std::enable_if<
		std::is_same_v<X, typename std::iterator_traits<Ix>::value_type> &&
		std::is_same_v<T, typename std::iterator_traits<It>::value_type> &&
		std::is_same_v<X(*)(const X&, const T&), typename callable_deducer<F>::signature_t>, void>
	{};
	/// <summary>
	/// Adams predictor/corrector of 8 degree
	/// </summary>
	/// <typeparam name="X"></typeparam>
	/// <typeparam name="T"></typeparam>
	template<typename X, typename T>
	class abm8impl {
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
		constexpr size_t degree() const { return 8; }
		/// <summary>
		/// Performes one step integration.
		/// </summary>
		/// <param name="ix">is iterator of collection of vectors</param>
		/// <param name="it">is iterator of collection of time values</param>
		/// <param name="step">is time step of integration</param>
		/// <param name="xk">is vector result</param>
		/// <param name="tk">is time result</param>
		/// <param name="func">is a callable function of right part of equations</param>
		/// <returns>no returns</returns>
		template<
			typename F, typename Ix, typename It,
			typename O = typename assignment_for_adams<X, T, std::remove_reference<F>::type, Ix, It>::type>
		void integrate(Ix ix, It it, double step, X& xk, T& tk, F&& func) const
		{
			xk = X{};
			auto xt{ func(*ix, *it) * _b[0] };
			X xb;
			for (size_t i{ 1 }; i < 8; ++i) {
				++it;
				++ix;
				xb = func(*ix, *it);
				xt += xb * _b[i];		// prediction
				xk += xb * _c[i - 1];	// correction
			}
			xt *= step;
			xt += (*ix);
			tk = (*it) + step;
			xk += func(xt, tk) * _c[7];
			xk *= step;
			xk += (*ix);
		}
	};
}