#pragma once
#include <integration.h>
#include <type_traits>

namespace math {
	template<typename X, typename T, typename F>
	struct assignment_for_rk : public std::enable_if<
		std::is_same_v<X(*)(const X&, const T&), typename callable_deducer<F>::signature_t>, void>
	{};

	/// <summary>
	/// Runge-Kutta explicit integrator of 4th degree
	/// </summary>
	/// <typeparam name="X">is dependent type X = X(t)</typeparam>
	/// <typeparam name="T">is independent type</typeparam>
	template<typename X, typename T>
	class rk4expl {
	public:
		/// <summary>
		/// Performes one step integration.
		/// </summary>
		/// <param name="x0">is initial vector</param>
		/// <param name="t0">is initial time</param>
		/// <param name="step">is step of integration</param>
		/// <param name="xk">is vector result</param>
		/// <param name="tk">is time value result</param>
		/// <param name="func">is callable function of right part of equations</param>
		/// <returns>no returns</returns>
		template<
			typename F, 
			typename O = typename assignment_for_rk<X, T, std::remove_reference<F>::type>::type>
		void integrate(const X& x0, const T& t0, double step, X& xk, T& tk, F&& func) const 
		{
			double step_2 = 0.5 * step, step_6 = step / 6.0;
			auto t = t0 + step_2;
			tk = t0 + step;
			auto k1{ func(x0, t0) };
			auto k2{ func(x0 + k1 * step_2, t) };
			auto k3{ func(x0 + k2 * step_2, t) };
			auto k4{ func(x0 + k3 * step,  tk) };
			xk = x0 + (k1 + (k2 + k3) * 2.0 + k4) * step_6;
		}
	};
}