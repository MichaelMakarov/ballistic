#pragma once
#include <egm.h>

namespace ball {

	struct trigonometric_func {
		double cos, sin;
	};

	class geopotential {
	private:
		std::vector<trigonometric_func> _cs;
		std::vector<double> _pnm;
		const potential_harmonic* _harmonics{};
		size_t _count{};
		double _rad{}, _mu{};

		void calc_trigonometric(double coslambda, double sinlambda);
		void calc_polynoms(double cosphi, double sinphi);
		
		void move(geopotential& other) noexcept;
	public:
		geopotential() = default;
		template<egm_type type>
		geopotential(egm_constants<type>, size_t count);
		geopotential(const geopotential& other) = default;
		geopotential(geopotential&& other) noexcept;
		~geopotential() noexcept = default;

		geopotential& operator= (const geopotential& other) = default;
		geopotential& operator= (geopotential&& other) noexcept;

		/// <summary>
		/// Computes the value of geopotential
		/// </summary>
		/// <param name="point"> - a point in GCS where to calculate geopotential</param>
		/// <returns>geopotential value</returns>
		double operator() (double x, double y, double z);
		/// <summary>
		/// Calculating the potential acceleration with respect to geocentric cartesian coordinates 
		/// </summary>
		/// <param name="point"> - a point in GCS where to calculate the geopotential</param>
		/// <returns>a vector of partial acceleration (dU/dx, dU/dy, dU/dz)</returns>
		XYZ acceleration(double x, double y, double z);
	};

	template<egm_type type>
	inline geopotential::geopotential(egm_constants<type>, size_t count) {
		_harmonics = egm_constants<type>::harmonics.data();
		_count = std::min(count, egm_constants<type>::count);
		_rad = egm_constants<type>::rad;
		_mu = egm_constants<type>::mu;
		size_t dim = ((_count + 1) * (_count + 2)) / 2;
		_cs.resize(_count + 1);
		_pnm.resize(dim + 2);
		_pnm[dim] = _pnm[dim + 1] = 0.0;
	}
}