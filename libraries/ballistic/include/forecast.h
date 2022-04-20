#pragma once
#include <structures.h>
#include <runge_kutta_integrator.h>
#include <adams_integrator.h>
#include <algorithm>
#include <vector>

namespace ball {
	/// <summary>
	/// Interface stores data of motion and provides methods to compute parameters of motion (dim is vector dimension)
	/// </summary>
	template<size_t dim> 
	class forecast	{
		std::vector<math::vec<dim>> _trajectory;
		std::vector<JD> _times;
		std::vector<size_t> _loops;
		double _deltatime;
	private:
		void move(forecast&& other) noexcept;
	public:
		forecast() : _deltatime{ } {}
		forecast(const forecast& other) = default;
		forecast(forecast&& other) noexcept;
		~forecast() = default;

		forecast& operator= (const forecast& other) = default;
		forecast& operator= (forecast&& other) noexcept;
		
		/// <summary>
		/// Returns array of vectors
		/// </summary>
		/// <returns></returns>
		const std::vector<math::vec<dim>>& get_points() const noexcept;
		/// <summary>
		/// Returns array of julian date values
		/// </summary>
		/// <returns></returns>
		const std::vector<JD>& get_times() const noexcept;
		/// <summary>
		/// Returns array of loop numbers
		/// </summary>
		/// <returns></returns>
		const std::vector<size_t>& get_loops() const noexcept;
		/// <summary>
		/// Computes parameters of motion for specified julian date (degree is polynomial degree of approximation).
		/// </summary>
		/// <param name="time">is julian date refered to midnight</param>
		/// <returns></returns>
		template<size_t degree = 4>	
		motion_params<dim> get_point(const JD& time) const;
		/// <summary>
		/// Computes parameters of motion.
		/// </summary>
		/// <typeparam name="F">is function invoker type</typeparam>
		/// <param name="x0">is initial parameters of motion</param>
		/// <param name="tk">is initial julian date</param>
		/// <param name="func">is right part of equations of motion</param>
		/// <param name="deltasec">is time step for integration in seconds</param>
		template<typename F>
		void run(const motion_params<dim>& x0, const JD& tk,	F&& func, double deltasec = 30);

	private:
		template<typename F>
		void start_run(size_t index, const math::rk4expl<math::vec<dim>, JD>& integrator, F&& func);
		template<typename F>
		void continue_run(size_t index, const math::abm8impl<math::vec<dim>, JD>& integrator, F&& func);
	};
		
	template<size_t dim>
	inline void forecast<dim>::move(forecast<dim>&& other) noexcept {
		_trajectory = std::move(other._trajectory);
		_times = std::move(other._times);
		_loops = std::move(other._loops);
		std::swap(_deltatime, other._deltatime);
	}

	template<size_t dim>
	inline forecast<dim>::forecast(forecast<dim>&& other) noexcept {
		move(std::forward<forecast<dim>>(other));
	}

	template<size_t dim>
	inline forecast<dim>& forecast<dim>::operator=(forecast<dim>&& other) noexcept {
		move(std::forward<forecast<dim>>(other));
		return *this;
	}

	template<size_t dim>
	inline const std::vector<math::vec<dim>>& forecast<dim>::get_points() const noexcept {
		return _trajectory;
	}

	template<size_t dim>
	inline const std::vector<JD>& forecast<dim>::get_times() const noexcept {
		return _times;
	}

	template<size_t dim>
	inline const std::vector<size_t>& forecast<dim>::get_loops() const noexcept {
		return _loops;
	}

	template<size_t dim>
	template<size_t degree>
	inline motion_params<dim> forecast<dim>::get_point(const JD& time) const {
		auto count = _trajectory.size();
		if (count < degree) {
			throw std::length_error("not enough points of trajectory for approximation");
		}
		auto index = static_cast<size_t>((time - _times[0]) / _deltatime);
		if (index >= count) {
			throw std::invalid_argument("time out of bounds");
		}
		// checking the value sign of the z coordinate of the nearest point
		bool intersection = std::signbit(_trajectory[index][2]);
		size_t loop{ _loops[index] };
		// get the index of the first point for the interpolation
		index = std::max(int{}, std::min(int(count) - int(degree), int(index) - int(degree / 2)));
		// hence the interpolation performing
		// P(t) = sum{n = 0..dim} (mult{r = 0..dim, r != n} (t - t_m)/(t_n - t_m)) x_n
		math::vec<dim> result;
		double mult;
		size_t indexn{ index };
		for (size_t n{}; n < degree; ++n) {
			mult = 1.0;
			for (size_t k{}; k < degree; ++k) {
				if (k != n) {
					mult *= (time - _times[index + k]) / (_times[indexn] - _times[index + k]);
				}
			}
			result += mult * _trajectory[indexn++];
		}
		intersection &= !std::signbit(result[2]);
		if (intersection) ++loop;
		return motion_params<dim>{ result, time, loop };
	}

	template<size_t dim>
	template<typename F>
	inline void forecast<dim>::run(const motion_params<dim>& x0, const JD& tk, F&& func, double deltasec) 
	{
		if (deltasec < 0.0) {
			throw std::invalid_argument("invalid deltatime");
		}
		if (tk < x0.jd + deltasec) {
			throw std::invalid_argument("invalid tk < tn + deltatime");
		}
		math::rk4expl<math::vec<dim>, JD> singlestep_int;
		math::abm8impl<math::vec<dim>, JD> multistep_int;
		size_t index = multistep_int.degree() - 1;
		size_t count = size_t((tk - x0.jd) / deltasec) + 1;
		_trajectory.resize(count);
		_times.resize(count);
		_loops.resize(count);
		_trajectory[0] = x0.vec;
		_times[0] = x0.jd;
		_loops[0] = x0.loop;
		_deltatime = deltasec;
		// performing initial calculations
		start_run(index, singlestep_int, func);
		// performing remaining calculations
		continue_run(index, multistep_int, func);
	}

	template<size_t dim>
	template<typename F>
	inline void forecast<dim>::start_run(size_t index, const math::rk4expl<math::vec<dim>, JD>& integrator, F&& func) 
	{
		auto x0{ _trajectory[0] };
		auto t0{ _times[0] };
		math::vec<dim> xk;
		JD tk;
		bool intersection;
		size_t n{ 6 };
		double step{ _deltatime / n };
		for (size_t i{}; i < index; ++i) {
			_loops[i + 1] = _loops[i];
			for (size_t k{}; k < n; ++k) {
				integrator.integrate(x0, t0, step, xk, tk, func);
				intersection = std::signbit(x0[2]) && !std::signbit(xk[2]);
				x0 = xk;
				t0 = tk;
				if (intersection) ++_loops[i + 1];
			}
			_trajectory[i + 1] = xk;
			_times[i + 1] = tk;
		}
	}

	template<size_t dim>
	template<typename F>
	inline void forecast<dim>::continue_run(size_t index, const math::abm8impl<math::vec<dim>, JD>& integrator, F&& func) {
		bool intersection;
		auto ix{ _trajectory.begin() };
		auto it{ _times.begin() };
		for (size_t i{ index }; i < _trajectory.size() - 1; ++i) {
			integrator.integrate(ix, it, _deltatime, _trajectory[i + 1], _times[i + 1], func);
			intersection = std::signbit(_trajectory[i][2]) && !std::signbit(_trajectory[i + 1][2]);
			_loops[i + 1] = _loops[i] + intersection ? 1 : 0;
			++ix;
			++it;
		}
	}

}
