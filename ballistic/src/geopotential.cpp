#include <geopotential.h>
#include <static_matrix.h>

namespace ball {
	void geopotential::calc_trigonometric(double coslambda, double sinlambda) {
		_cs[0].cos = 1.0; _cs[0].sin = 0.0;
		for (size_t i = 1; i <= _count; ++i) {
			_cs[i].cos = _cs[i - 1].cos * coslambda - _cs[i - 1].sin * sinlambda;
			_cs[i].sin = _cs[i - 1].sin * coslambda + _cs[i - 1].cos * sinlambda;
		}
	}

	void geopotential::calc_polynoms(double cosphi, double sinphi) {
		_pnm[0] = 1;
		_pnm[1] = sinphi * std::sqrt(3);
		_pnm[2] = cosphi * std::sqrt(3);
		size_t k{ 3 };
		for (size_t n = 2; n <= _count; ++n) {
			for (size_t m = 0; m < n; ++m) {
				_pnm[k] = std::sqrt(2 * n - 1) * sinphi * _pnm[k - n] - std::sqrt((n - 1 - m) * (n - 1 + m) / (2.0 * n - 3)) * _pnm[(k + 1) - n - n];
				_pnm[k++] *= std::sqrt((2.0 * n + 1) / ((n - m) * (n + m)));
			}
			_pnm[k++] = std::sqrt(1 + 0.5 / n) * cosphi * _pnm[k - n - 1];
		}
	}

	void geopotential::move(geopotential& other) noexcept {
		std::swap(_rad, other._rad);
		std::swap(_mu, other._mu);
		std::swap(_count, other._count);
		std::swap(_harmonics, other._harmonics);
		_cs = std::move(other._cs);
		_pnm = std::move(other._pnm);
	}

	geopotential::geopotential(geopotential&& other) noexcept {
		move(other);
	}

	geopotential& geopotential::operator= (geopotential&& other) noexcept {
		move(other);
		return *this;
	}

	double geopotential::operator() (double x, double y, double z) {
		double result{ 0 };
		double r = std::sqrt(x * x + y * y + z * z);
		double xy = std::sqrt(x * x + y * y);
		double sinphi{ z / r };
		double cosphi{ xy / r };
		double coslambda{ x / xy };
		double sinlambda{ y / xy };
		double R_r{ _rad / r };
		double mult{ 1 };
		size_t k{ 0 };
		calc_trigonometric(coslambda, sinlambda);
		calc_polynoms(cosphi, sinphi);
		for (size_t n = 0; n <= _count; ++n) {
			for (size_t m = 0; m <= n; ++m) {
				result += mult * _pnm[k] * (_harmonics[k].cos * _cs[m].cos + _harmonics[k].sin * _cs[m].sin);
				k++;
			}
			mult *= R_r;
		}
		return _mu / r * result;
	}

	constexpr inline double delta(size_t m) noexcept { 
		if (m == 0) return 0.5;
		else return 1.0;
	}
	double dpnm(double pnm, double pnm1, size_t n, size_t m, double tgphi) noexcept	{
		return -pnm * tgphi * m + pnm1 * (m < n) * std::sqrt((n - m) * (n + m + 1) * delta(m));
	}

	XYZ geopotential::acceleration(double x, double y, double z) {
		using namespace math;
		
		double r = std::sqrt(x * x + y * y + z * z);
		double xy = std::sqrt(x * x + y * y);
		double sinphi{ z / r };
		double cosphi{ xy / r };
		double tgphi{ sinphi / cosphi };
		double coslambda{ x / xy };
		double sinlambda{ y / xy };
		double mu_r2{ _mu / r / r };
		double R_r{ _rad / r };
		double mult{ 1 };
		mat3x3 ct{
			{
				{ cosphi * coslambda, -sinphi * coslambda, -sinlambda },
				{ cosphi * sinlambda, -sinphi * sinlambda, coslambda },
				{ sinphi,				cosphi,				0 }
			} 
		};

		size_t k{ 0 };
		double kcs, ksc, poly;
		vec3 dUn, dUsum;
		
		calc_trigonometric(coslambda, sinlambda);
		calc_polynoms(cosphi, sinphi);

		// calculating the potential acceleration
		for (size_t n = 0; n <= _count; ++n) {
			dUn[0] = dUn[1] = dUn[2] = 0.0;
			for (size_t m = 0; m <= n; ++m) {
				poly = _pnm[k];
				// Cnm * cos(r * L) + Snm * sin(r * L)
				kcs = _harmonics[k].cos * _cs[m].cos + _harmonics[k].sin * _cs[m].sin;
				// Snm * cos(r * L) - Cnm * sin(r * L)
				ksc = _harmonics[k].sin * _cs[m].cos - _harmonics[k].cos * _cs[m].sin;
				dUn[0] -= poly * kcs;										// a derivative by r
				dUn[1] += dpnm(_pnm[k], _pnm[k + 1], n, m, tgphi) * kcs;	// a derivative by phi
				dUn[2] += poly * ksc * m;									// a derivative by lambda
				++k;
			}
			dUsum[0] += (n + 1) * mult * dUn[0];
			dUsum[1] += mult * dUn[1];
			dUsum[2] += mult * dUn[2];
			mult *= R_r;
		}
		dUsum[2] /= cosphi;
		dUsum *= mu_r2;
		return ct * dUsum;
	}

	//std::pair<general::vec3, general::matrix3x3> geopotential::derivatives(const general::vec3& point) {
	//	using namespace general;

	//	const double r{ point.length() };
	//	const double xy{ std::sqrt(point[0] * point[0] + point[1] * point[1]) };
	//	const double sinphi{ point[2] / r };
	//	const double cosphi{ xy / r }, cosphi2{ cosphi * cosphi };
	//	const double tgphi{ sinphi / cosphi };
	//	const double coslambda{ point[0] / xy };
	//	const double sinlambda{ point[1] / xy };
	//	const double mu_r2{ _eMu / r / r };
	//	const double R_r{ _eR / r };
	//	double mult{ 1 };
	//	const matrix3x3 CT{
	//		{
	//			{  cosphi * coslambda,	-sinphi * coslambda,	-sinlambda },
	//			{  cosphi * sinlambda,	-sinphi * sinlambda,	 coslambda },
	//			{  sinphi,				 cosphi,				 0 }
	//		}
	//	}; 
	//	const matrix3x3 C{
	//		{
	//			{  cosphi * coslambda,	 cosphi * sinlambda,	 sinphi },
	//			{ -sinphi * coslambda,	-sinphi * sinlambda,	 cosphi },
	//			{ -sinlambda,			 coslambda,				 0 }
	//		}
	//	};
	//	matrix3x3 G;
	//	size_t k{ 0 };
	//	double kcs, ksc, dpoly, poly;
	//	vec3 dUn, dUsum, ddUn;
	//	vec<6> ddUsum;

	//	calc_trigonometric(coslambda, sinlambda);
	//	calc_polynoms(cosphi, sinphi);

	//	// calculating the potential acceleration
	//	for (size_t n = 0; n <= _count; ++n) {
	//		dUn[0] = dUn[1] = dUn[2] = 0.0;
	//		ddUn[0] = ddUn[1] = ddUn[2] = 0.0;
	//		for (size_t m = 0; m <= n; ++m) {
	//			poly = _pnm[k];
	//			dpoly = dpnm(_pnm[k], _pnm[k + 1], n, m, tgphi);
	//			// Cnm * cos(r * L) + Snm * sin(r * L)
	//			kcs = _harmonics[k].cos * _cs[m].cos + _harmonics[k].sin * _cs[m].sin;
	//			// Snm * cos(r * L) - Cnm * sin(r * L)
	//			ksc = _harmonics[k].sin * _cs[m].cos - _harmonics[k].cos * _cs[m].sin;
	//			dUn[0] -= poly * kcs;		// a derivative by r
	//			dUn[1] += dpoly * kcs;		// a derivative by phi
	//			dUn[2] += poly * ksc * m;	// a derivative by lambda
	//			// a double derivative by phi
	//			ddUn[0] += (dpnm(_pnm[k + 1], _pnm[k + 2], n, m + 1, tgphi) - m * (poly / cosphi2 + dpoly * tgphi)) * kcs;
	//			ddUn[1] += dpoly * ksc * m;	// a derivative by phi and lambda
	//			ddUn[2] = dUn[0] * m * m;	// a double derivative by lambda
	//			++k;
	//		}
	//		dUsum[0] += (n + 1) * mult * dUn[0];
	//		dUsum[1] += mult * dUn[1];
	//		dUsum[2] += mult * dUn[2];
	//		ddUsum[0] -= (n + 2) * (n + 1) * mult * dUn[0];	// a double derivative by r
	//		ddUsum[1] -= (n + 1) * mult * dUn[1];			// a derivative by r and phi
	//		ddUsum[2] -= (n + 1) * mult * dUn[2];			// a derivative by r and lambda
	//		ddUsum[3] += mult * ddUn[0];					// a double derivative by phi
	//		ddUsum[4] += mult * ddUn[1];					// a derivative by phi and lambda
	//		ddUsum[5] += mult * ddUn[2];					// a double derivative by lambda
	//		mult *= R_r;
	//	}
	//	dUsum[2] /= cosphi;
	//	dUsum *= mu_r2;
	//	ddUsum *= mu_r2 / r;
	//	ddUsum[2] /= cosphi;
	//	ddUsum[4] /= cosphi;
	//	ddUsum[5] /= cosphi * cosphi;
	//	// matrix of partial acceleration filling
	//	G(0, 0) = ddUsum[0];
	//	G(0, 1) = G(1, 0) = ddUsum[1] - dUsum[1] / r;
	//	G(0, 2) = G(2, 0) = ddUsum[2] - dUsum[2] / r;
	//	G(1, 1) = dUsum[0] / r + ddUsum[3];
	//	G(1, 2) = G(2, 1) = tgphi * dUsum[2] / r + ddUsum[4];
	//	G(2, 2) = (dUsum[0] - tgphi * dUsum[1]) / r + ddUsum[5];

	//	return std::make_pair(CT * dUsum, CT * G * C);
	//}
}