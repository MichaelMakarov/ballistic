#include <ball.h>
#include <arithmetics.h>
#include <linalg.h>

std::vector<potential_harmonic> EGM96::harmonics;
std::vector<potential_harmonic> JGM3::harmonics;

std::ostream& operator<<(std::ostream& out, const potential_harmonic& p)
{
	return out << p.cos << ' ' << p.sin;
}

std::istream& operator>> (std::istream& in, potential_harmonic& p)
{
	return in >> p.cos >> p.sin;
}

void mass_acceleration(const double* const p, const double* const m, double mu, double* const a)
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

double height_above_ellipsoid(const double* const v, double r, double f)
{
	double dist = std::sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	return dist - r * (1 - f * sqr(v[2] / dist));
}

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

double geopotential::operator() (const double* const v)
{
	auto x = v[0], y = v[1], z = v[2];
	double result{ 0 };
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
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

void geopotential::acceleration(const double* const in, double* const out)
{
	double x = in[0], y = in[1], z = in[2];
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinphi{ z / r };
	double cosphi{ xy / r };
	double tgphi{ sinphi / cosphi };
	double coslambda{ x / xy };
	double sinlambda{ y / xy };
	double mu_r2{ _mu / r / r };
	double R_r{ _rad / r };
	double mult{ 1 };
	mat3x3 ct{
		cosphi * coslambda, -sinphi * coslambda, -sinlambda,
		cosphi * sinlambda, -sinphi * sinlambda,  coslambda,
		sinphi,				 cosphi,				0
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
	dUsum = ct * dUsum;

	for (size_t i{}; i < 3; ++i) out[i] = dUsum[i];
}
