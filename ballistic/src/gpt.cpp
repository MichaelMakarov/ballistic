#include <ball.hpp>
#include <maths.hpp>

namespace egm {
    std::vector<potential_harmonic> harmonics;
}


void mass_acceleration(const double* p, const double* m, double mu, double* a)
{
	double d[3];
	for (size_t i{}; i < 3; ++i) d[i] = m[i] - p[i];
	double mlen{}, dlen{};
	for (size_t i{}; i < 3; ++i) {
		mlen += sqr(m[i]);
		dlen += sqr(d[i]);
	}
	mlen = cube(std::sqrt(mlen));
	dlen = cube(std::sqrt(dlen));
	for (size_t i{}; i < 3; ++i) a[i] = mu * (d[i] / dlen - m[i] / mlen);
}

double height_above_ellipsoid(const double* v, double r, double f)
{
	double dist = std::sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	return dist - r * (1 - f * sqr(v[2] / dist));
}

/**
 * @brief Вычисление гармоник синусов и косинусов долготы.
 */
void calc_trigonometric(double cos, double sin, trigonometric_func* cs, size_t count) {
    cs[0].cos = 1; cs[0].sin = 0;
    for (size_t i{ 1 }; i <= count; ++i) {
        cs[i].cos = cs[i - 1].cos * cos - cs[i - 1].sin * sin;
        cs[i].sin = cs[i - 1].sin * cos + cs[i - 1].cos * sin;
    }
}

/**
 * @brief Вычисление значений полиномов Лежандра.
 */
void calc_polynomials(double cos, double sin, double* pnm, size_t count) {
    pnm[0] = 1;
    pnm[1] = sin * std::numbers::sqrt3;
    pnm[2] = cos * std::numbers::sqrt3;
    size_t k{ 3 };
    for (size_t n = 2; n <= count; ++n) {
        for (size_t m = 0; m < n; ++m) {
            pnm[k] = std::sqrt(2 * n - 1) * sin * pnm[k - n] - std::sqrt((n - 1 - m) * (n - 1 + m) / (2.0 * n - 3)) * pnm[(k + 1) - n - n];
            pnm[k++] *= std::sqrt((2.0 * n + 1) / ((n - m) * (n + m)));
        }
        pnm[k++] = std::sqrt(1 + 0.5 / n) * cos * pnm[k - n - 1];
    }
}

geopotential::geopotential(size_t count)
{
    count = std::min(count, egm::count);
    size_t dim = ((count + 1) * (count + 2)) / 2;
    _cs.resize(count + 1);
    _pnm.resize(dim + 2);
    _pnm[dim] = _pnm[dim + 1] = 0;
}

void geopotential::move(geopotential& other) noexcept
{
	_cs.swap(other._cs);
	_pnm.swap(other._pnm);
}

geopotential::geopotential(geopotential&& other) noexcept {
	move(other);
}

geopotential& geopotential::operator=(geopotential&& other) noexcept {
	move(other);
	return *this;
}

double geopotential::operator() (const double* v)
{
	auto x = v[0], y = v[1], z = v[2];
	double result{ 0 };
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinphi{ z / r };
	double cosphi{ xy / r };
	double coslambda{ x / xy };
	double sinlambda{ y / xy };
	double R_r{ egm::rad / r };
	double mult{ 1 };
	size_t k{ 0 };
    size_t count = _cs.size() - 1;
	calc_trigonometric(coslambda, sinlambda, _cs.data(), count);
	calc_polynomials(cosphi, sinphi, _pnm.data(), count);
	for (size_t n = 0; n <= count; ++n) {
		for (size_t m = 0; m <= n; ++m) {
			result += mult * _pnm[k] * (egm::harmonics[k].cos * _cs[m].cos + egm::harmonics[k].sin * _cs[m].sin);
			k++;
		}
		mult *= R_r;
	}
	return egm::mu / r * result;
}

constexpr inline double delta(size_t m) noexcept {
	if (m == 0) return 0.5;
	return 1.0;
}
double dpnm(double pnm, double pnm1, size_t n, size_t m, double tgphi) noexcept	{
	return -pnm * tgphi * m + pnm1 * (m < n) * std::sqrt((n - m) * (n + m + 1) * delta(m));
}

void geopotential::acceleration(const double* in, double* out)
{
	double x = in[0], y = in[1], z = in[2];
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinphi{ z / r };
	double cosphi{ xy / r };
	double tgphi{ sinphi / cosphi };
	double coslambda{ x / xy };
	double sinlambda{ y / xy };
	double mu_r2{ egm::mu / r / r };
	double R_r{ egm::rad / r };
	double mult{ 1 };
	mat3x3 ct{
		{ cosphi * coslambda, -sinphi * coslambda, -sinlambda },
		{ cosphi * sinlambda, -sinphi * sinlambda,  coslambda },
		{ sinphi,				 cosphi,				0	  }
	};

	size_t k{ 0 };
    size_t count = _cs.size() - 1;
	vec3 dUn, dUsum;

    calc_trigonometric(coslambda, sinlambda, _cs.data(), count);
    calc_polynomials(cosphi, sinphi, _pnm.data(), count);

	// calculating the potential acceleration
	for (size_t n = 0; n <= count; ++n) {
		dUn[0] = dUn[1] = dUn[2] = 0.0;
		for (size_t m = 0; m <= n; ++m) {
			auto poly = _pnm[k];
			// Cnm * cos(r * L) + Snm * sin(r * L)
			auto kcs = egm::harmonics[k].cos * _cs[m].cos + egm::harmonics[k].sin * _cs[m].sin;
			// Snm * cos(r * L) - Cnm * sin(r * L)
			auto ksc = egm::harmonics[k].sin * _cs[m].cos - egm::harmonics[k].cos * _cs[m].sin;
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
	dUsum = ct * dUsum;

	for (size_t i{}; i < 3; ++i) out[i] = dUsum[i];
}
