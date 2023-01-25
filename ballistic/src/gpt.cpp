#include <ball.hpp>
#include <maths.hpp>
#include <cmath>

using namespace math;

namespace egm
{
	std::vector<potential_harmonic> harmonics;
}

void mass_acceleration(const double *p, const double *m, double mu, double *a)
{
	double d[3];
	double mlen{}, dlen{};
	for (size_t i{}; i < 3; ++i)
	{
		d[i] = m[i] - p[i];
		mlen += sqr(m[i]);
		dlen += sqr(d[i]);
	}
	mlen = cube(std::sqrt(mlen));
	dlen = cube(std::sqrt(dlen));
	for (size_t i{}; i < 3; ++i)
		a[i] = mu * (d[i] / dlen - m[i] / mlen);
}

double height_above_ellipsoid(const double *v, double r, double f)
{
	double dist = std::sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	return dist - r * (1 - f * sqr(v[2] / dist));
}

/**
 * @brief Вычисление гармоник синусов и косинусов долготы.
 */
void calc_trigonometric(double cos, double sin, trigonometric_func *cs, size_t count)
{
	cs[0].cos = 1;
	cs[0].sin = 0;
	for (size_t i{1}; i <= count; ++i)
	{
		cs[i].cos = cs[i - 1].cos * cos - cs[i - 1].sin * sin;
		cs[i].sin = cs[i - 1].sin * cos + cs[i - 1].cos * sin;
	}
}

/**
 * @brief Вычисление значений полиномов Лежандра.
 */
void calc_polynomials(double cos, double sin, double *pnm, size_t count)
{
	double const sqrt3 = std::sqrt(3);
	pnm[0] = 1;
	pnm[1] = sin * sqrt3;
	pnm[2] = cos * sqrt3;
	for (size_t n{2}, k{3}; n <= count; ++n)
	{
		for (size_t m{}; m < n; ++m, ++k)
		{
			pnm[k] = std::sqrt(2 * n - 1) * sin * pnm[k - n] - std::sqrt((n - 1 - m) * (n - 1 + m) / (2. * n - 3)) * pnm[(k + 1) - n - n];
			pnm[k] *= std::sqrt((2. * n + 1) / ((n - m) * (n + m)));
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

void geopotential::move(geopotential &other) noexcept
{
	_cs.swap(other._cs);
	_pnm.swap(other._pnm);
}

geopotential::geopotential(geopotential &&other) noexcept
{
	move(other);
}

geopotential &geopotential::operator=(geopotential &&other) noexcept
{
	move(other);
	return *this;
}

double geopotential::operator()(const double *v)
{
	auto x = v[0], y = v[1], z = v[2];
	double result{0};
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinf{z / r};
	double cosf{xy / r};
	double cosl{x / xy};
	double sinl{y / xy};
	double R_r{egm::rad / r};
	double mult{1};
	size_t k{0};
	size_t count = _cs.size() - 1;
	calc_trigonometric(cosl, sinl, _cs.data(), count);
	calc_polynomials(cosf, sinf, _pnm.data(), count);
	for (size_t n = 0; n <= count; ++n)
	{
		for (size_t m = 0; m <= n; ++m)
		{
			result += mult * _pnm[k] * (egm::harmonics[k].cos * _cs[m].cos + egm::harmonics[k].sin * _cs[m].sin);
			k++;
		}
		mult *= R_r;
	}
	return egm::mu / r * result;
}

constexpr inline double delta(size_t m) noexcept
{
	return m == 0 ? 0.5 : 1.0;
}
double dpnm(double pnm, double pnm1, size_t n, size_t m, double tanf) noexcept
{
	double out = -pnm * tanf * m;
	if (m < n)
		out += pnm1 * std::sqrt(delta(m) * (n + m + 1) * (n - m));
	return out;
}

double ddpnm(double pnm, double pnm1, double pnm2, std::size_t n, std::size_t m, double tanf, double cosf)
{
	double out = (sqr(tanf) * m - 1 / sqr(cosf)) * pnm;
	if (m < n)
	{
		double mul = (n - m) * (n + m + 1) * delta(m);
		out -= pnm1 * std::sqrt(mul) * (m + m + 1) * tanf;
		if (m < n - 1)
			out += pnm2 * std::sqrt(mul * (n + m + 2) * (n - m - 1));
	}
	return out;
}

void geopotential::acceleration(const double *in, double *out)
{
	double x = in[0], y = in[1], z = in[2];
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinf{z / r};
	double cosf{xy / r};
	double tanf{sinf / cosf};
	double cosl{x / xy};
	double sinl{y / xy};
	double mu_r2{egm::mu / (r * r)};
	double R_r{egm::rad / r};
	double mult{1};
	mat3x3 mx{
		{cosf * cosl, -sinf * cosl, -sinl},
		{cosf * sinl, -sinf * sinl, cosl},
		{sinf, cosf, 0}};
	size_t k{0};
	size_t count = _cs.size() - 1;
	vec3 du, dus;

	calc_trigonometric(cosl, sinl, _cs.data(), count);
	calc_polynomials(cosf, sinf, _pnm.data(), count);

	// calculating the potential acceleration
	for (size_t n = 0; n <= count; ++n)
	{
		du[0] = du[1] = du[2] = 0.0;
		for (size_t m = 0; m <= n; ++m)
		{
			auto poly = _pnm[k];
			// Cnm * cos(r * L) + Snm * sin(r * L)
			auto kcs = egm::harmonics[k].cos * _cs[m].cos + egm::harmonics[k].sin * _cs[m].sin;
			// Snm * cos(r * L) - Cnm * sin(r * L)
			auto ksc = egm::harmonics[k].sin * _cs[m].cos - egm::harmonics[k].cos * _cs[m].sin;
			// производная по радиусу-вектору
			du[0] -= poly * kcs;
			// производная по широте
			du[1] += dpnm(_pnm[k], _pnm[k + 1], n, m, tanf) * kcs;
			// производная по долготе
			du[2] += poly * ksc * m;
			++k;
		}
		dus[0] += (n + 1) * mult * du[0];
		dus[1] += mult * du[1];
		dus[2] += mult * du[2];
		mult *= R_r;
	}
	dus[2] /= cosf;
	dus *= mu_r2;
	dus = mx * dus;

	for (size_t i{}; i < 3; ++i)
		out[i] = dus[i];
}
/**
 * @brief Вычисление матрицы производных сферических координат по декартовым координатам.
 *
 */
mat3x3 coordinates_deriv(double r, double cosf, double sinf, double cosl, double sinl)
{
	double _r{1 / r};
	double _rcosf{_r / cosf};
	mat3x3 m;
	// dr/dx
	m[0][0] = cosf * cosl;
	// dr/dy
	m[0][1] = cosf * sinl;
	// dr/dz
	m[0][2] = sinf;
	// df/dx
	m[1][0] = -sinf * cosl * _r;
	// df/dy
	m[1][1] = -sinf * sinl * _r;
	// df/dz
	m[1][2] = cosf * _r;
	// dl/dx
	m[2][0] = -sinl * _rcosf;
	// dl/dy
	m[2][1] = cosl * _rcosf;
	return m;
}
/**
 * @brief Вычисление матрицы вторых производных от радиуса по x, y, z.
 *
 */
mat3x3 radius_deriv(double r, double drdx, double drdy, double drdz)
{
	mat3x3 m;
	// ddr/ddx
	m[0][0] = 1 - sqr(drdx);
	// ddr/ddy
	m[1][1] = 1 - sqr(drdy);
	// ddr/ddz
	m[2][2] = 1 - sqr(drdz);
	// ddr/dxdy
	m[0][1] = m[1][0] = -drdx * drdy;
	// ddr/dxdz
	m[0][2] = m[2][0] = -drdx * drdz;
	// ddr/dydz
	m[1][2] = m[2][1] = -drdy * drdz;
	m *= (1 / r);
	return m;
}
/**
 * @brief Вычисление матрицы вторых производных от долготы по x, y, z.
 *
 */
mat3x3 longitude_deriv(double rcosf, double cosl, double sinl)
{
	double rcosf_sqr = sqr(rcosf);
	mat3x3 m;
	// ddl/ddx
	m[0][0] = 2 * cosl * sinl / rcosf_sqr;
	// ddl/ddy
	m[1][1] = -m[0][0];
	// ddl/dxdy
	m[0][1] = m[1][0] = (sqr(cosl) - sqr(sinl)) / rcosf_sqr;
	return m;
}
/**
 * @brief Вычисление матрицы вторых производных от широты по x, y, z.
 *
 */
mat3x3 latitude_deriv(double r, double cosf, double sinf, double tanf, double cosl, double sinl)
{
	double _rsqr = 1 / sqr(r);
	double cos_bracket = 2 * sqr(cosf) + 1;
	double sin_bracket = 2 * sqr(sinf) - 1;
	mat3x3 m;
	// ddf/ddx
	m[0][0] = tanf * _rsqr * (sqr(cosl) * cos_bracket - 1);
	// ddf/ddy
	m[1][1] = tanf * _rsqr * (sqr(sinl) * cos_bracket - 1);
	// ddf/ddz
	m[2][2] = -2 * cosf * sinf * _rsqr;
	// ddf/dxdy
	m[0][1] = m[1][0] = tanf * cosl * sinl * _rsqr * cos_bracket;
	// ddf/dxdz
	m[0][2] = m[2][0] = cosl * _rsqr * sin_bracket;
	// ddf/dydz
	m[1][2] = m[2][1] = sinl * _rsqr * sin_bracket;
	return m;
}

void geopotential::acceleration(double const in[3], double outv[3], double outm[3][3])
{
	double x = in[0], y = in[1], z = in[2];
	double xy = std::sqrt(sqr(x) + sqr(y));
	double r = std::sqrt(sqr(xy) + sqr(z));
	double sinf{z / r}, cosf{xy / r}, tanf{sinf / cosf};
	double cosl{x / xy}, sinl{y / xy};
	double _r{1 / r};
	double _rsqr = sqr(_r);
	double R_r{egm::rad / r};
	double mult{1};
	// матрица производных сферических координат по декартовым координатам
	const mat3x3 drfl = coordinates_deriv(r, cosf, sinf, cosl, sinl);
	// транспонированная матрица
	const mat3x3 drfl_t = transpose(drfl);
	// матрица вторых производных радиуса-вектора
	const mat3x3 ddr = radius_deriv(r, drfl[0][0], drfl[0][1], drfl[0][2]);
	// матрица вторых производных широты
	const mat3x3 ddf = latitude_deriv(r, cosf, sinf, tanf, cosl, sinl);
	// матрица вторых производных долготы
	const mat3x3 ddl = longitude_deriv(xy, cosl, sinl);
	// вектор производных потенциала по координатам
	vec3 du;
	// матрицы вторых производных по координатам
	mat3x3 ddu;
	// старшая гармоника
	size_t count = _cs.size() - 1;

	calc_trigonometric(cosl, sinl, _cs.data(), count);
	calc_polynomials(cosf, sinf, _pnm.data(), count);

	for (size_t n{}, k{}; n <= count; ++n)
	{
		double dudr{}, dudf{}, dudl{};
		double dduddr{}, dduddf{}, dduddl{}, ddudrdf{}, ddudrdl{}, ddudfdl{};
		// Rn(r) = (R / r)^n / r
		double Rn = mult;
		// dRn(r) = (R / r)^n * -(n + 1) / r
		double dRn = -Rn * (n + 1);
		// ddRn(r) = (R / r)^n * (n + 1) * (n + 2)
		double ddRn = dRn * (n + 2);
		for (size_t m{}; m <= n; ++m, ++k)
		{
			double poly = _pnm[k];
			double cnm = egm::harmonics[k].cos;
			double snm = egm::harmonics[k].sin;
			double cosml = _cs[m].cos;
			double sinml = _cs[m].sin;
			// Lnm(l) = Cnm * cos(ml) + Snm * sin(ml)
			double Lnm = cnm * cosml + snm * sinml;
			// dLnm(l)/dl = m * (Snm * cos(ml) - Cnm * sin(ml))
			double dLnm = m * (snm * cosml - cnm * sinml);
			// ddLnm(l)/ddl = -m * m * Lnm(l)
			double ddLnm = -m * m * Lnm;
			// Fnm(f) = Pnm(sin(f))
			double Fnm = _pnm[k];
			// dFnm(f) = dPnm(sin(f))/df
			double dFnm = dpnm(Fnm, _pnm[k + 1], n, m, tanf);
			// ddFnm(f) = ddPnm(sin(f))/ddf
			double ddFnm = ddpnm(Fnm, _pnm[k + 1], _pnm[k + 2], n, m, tanf, cosf);
			dduddr = dudr += Fnm * Lnm;
			ddudrdf = dudf += dFnm * Lnm;
			ddudrdl = dudl += Fnm * dLnm;
			dduddf += ddFnm * Lnm;
			dduddl += Fnm * ddLnm;
			ddudfdl += dFnm * dLnm;
		}
		du[0] = dudr * dRn;
		du[1] = dudf * Rn;
		du[2] = dudl * Rn;
		ddu[0][0] = dduddr * ddRn;
		ddu[0][1] = ddudrdf * dRn;
		ddu[0][2] = ddudrdl * dRn;
		ddu[1][1] = dduddf * Rn;
		ddu[2][2] = dduddl * Rn;
		ddu[1][2] = ddudfdl * Rn;
		mult *= R_r;
	}
	// умножаем на постоянную часть
	du[0] *= _rsqr;
	du[1] *= _r;
	du[2] *= _r;
	ddu[0][0] *= _rsqr * _r;
	ddu[0][1] *= _rsqr;
	ddu[0][2] *= _rsqr;
	ddu[1][1] *= _r;
	ddu[2][2] *= _r;
	ddu[1][2] *= _r;
	// умножаем на гравитационный множитель
	du *= egm::mu;
	ddu *= egm::mu;
	// дозаполняем матрицу вторых производных
	ddu[1][0] = ddu[0][1];
	ddu[2][0] = ddu[0][2];
	ddu[2][1] = ddu[1][2];
	// перевод к системе координат x, y, z
	ddu = drfl_t * ddu * drfl;
	ddu += ddr * du[0];
	ddu += ddf * du[1];
	ddu += ddl * du[2];
	du = drfl_t * du;

	for (std::size_t r{}; r < 3; ++r)
	{
		outv[r] = du[r];
		for (std::size_t c{}; c < 3; ++c)
		{
			outm[r][c] = ddu[r][c];
		}
	}
}