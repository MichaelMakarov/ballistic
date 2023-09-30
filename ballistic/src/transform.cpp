#include <transform.hpp>
#include <maths.hpp>
#include <cmath>

using namespace math;

/**
 * Декартовы координаты ожидаются в порядке (x, y, z).
 * Сферические радиус-вектор, широта/склонение, долгота/восхождение (r, f, l).
 */

/**
 * @brief Преобразование из декартовой СК в сферическую СК.
 *
 * @param in декартовые коодинаты
 * @param out сферические координаты
 */
void ort_to_sph(const double in[3], double out[3])
{
	out[0] = std::sqrt(sqr(in[0]) + sqr(in[1]) + sqr(in[2]));
	out[1] = std::asin(in[2] / out[0]);
	out[2] = std::atan2(in[1], in[0]);
}
/**
 * @brief Преобразование из сферической СК в декартову СК.
 *
 * @param in сферические координаты
 * @param out декартовы координаты
 */
void sph_to_ort(const double in[3], double out[3])
{
	double rcos = in[0] * std::cos(in[1]);
	out[0] = rcos * std::cos(in[2]);
	out[1] = rcos * std::sin(in[2]);
	out[2] = in[0] * std::sin(in[1]);
}
/**
 * @brief Преобразование из декартовой СК в сферическую СК.
 *
 * @param xyz координаты (x, y, z)
 * @param in скорости (dx, dy, dz)
 * @param out скорости (dr, df, dl)
 */
void ort_to_sph(double const xyz[3], double const in[3], double out[3])
{
	double rcos_sqr = sqr(xyz[0]) + sqr(xyz[1]);
	double r = std::sqrt(rcos_sqr + sqr(xyz[2]));
	out[0] = (xyz[0] * in[0] + xyz[1] * in[1] + xyz[2] * in[2]) / r;
	out[1] = (in[2] - xyz[2] * out[0] / r) / std::sqrt(rcos_sqr);
	out[2] = (xyz[0] * in[1] - xyz[1] * in[0]) / rcos_sqr;
}
/**
 * @brief Преобразование из сферической СК в декартову СК.
 *
 * @param rfl координаты (r, f, l)
 * @param in скорости (dr, df, dl)
 * @param out скорости (dx, dy, dz)
 */
void sph_to_ort(double const rfl[3], double const in[3], double out[3])
{
	double r = rfl[0];
	double cosf = std::cos(rfl[1]), sinf = std::cos(rfl[1]);
	double cosl = std::cos(rfl[2]), sinl = std::sin(rfl[2]);
	out[0] = cosf * cosl * in[0] - sinf * cosl * r * in[1] - cosf * sinl * r * in[2];
	out[1] = cosf * sinl * in[0] - sinf * sinl * r * in[1] + cosf * cosl * r * in[2];
	out[2] = sinf * in[0] + cosf * in[1];
}

void abs_to_grw_ort(const double in[3], double t, double out[3])
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost + in[1] * sint;
	out[1] = in[1] * cost - in[0] * sint;
	out[2] = in[2];
}

void grw_to_abs_ort(const double in[3], double t, double out[3])
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost - in[1] * sint;
	out[1] = in[1] * cost + in[0] * sint;
	out[2] = in[2];
}

void grw_to_abs_sph(const double in[3], double t, double out[3])
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = fit_round<round_type::zdpi>(in[2] + t);
}

void abs_to_grw_sph(const double in[3], double t, double out[3])
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = fit_round<round_type::zdpi>(in[2] - t);
}

void ecl_to_abs(const double in[3], double e, double out[3])
{
	double sine = std::sin(e), cose = std::cos(e);
	out[0] = in[0];
	out[1] = in[1] * cose - in[2] * sine;
	out[2] = in[1] * sine + in[2] * cose;
}

void abs_to_ecl(const double in[3], double e, double out[3])
{
	double sine = std::sin(e), cose = std::cos(e);
	out[0] = in[0];
	out[1] = in[1] * cose + in[2] * sine;
	out[2] = in[1] * sine - in[2] * cose;
}

void abs_to_grw_ort(double const inc[3], double const inv[3], double t, double w, double outc[3], double outv[3])
{
	double sint = std::sin(t), cost = std::cos(t);
	outc[0] = inc[0] * cost + inc[1] * sint;
	outc[1] = inc[1] * cost - inc[0] * sint;
	outc[2] = inc[2];
	outv[0] = inv[0] * cost + inv[1] * sint + w * outc[1];
	outv[1] = inv[1] * cost - inv[0] * sint - w * outc[0];
	outv[2] = inv[2];
}

void grw_to_abs_ort(double const inc[3], double const inv[3], double t, double w, double outc[3], double outv[3])
{
	double sint = std::sin(t), cost = std::cos(t);
	outc[0] = inc[0] * cost - inc[1] * sint;
	outc[1] = inc[1] * cost + inc[0] * sint;
	outc[2] = inc[2];
	outv[0] = inv[0] * cost - inv[1] * sint - w * outc[1];
	outv[1] = inv[1] * cost + inv[0] * sint + w * outc[0];
	outv[2] = inv[2];
}

void GCS_to_OCS(const double *const in, double **const out)
{
	vec3 r{in[0], in[1], in[2]};
	r.normalize();
	for (size_t i{}; i < 3; ++i)
		out[0][i] = r[i];
	vec3 l{-in[3], -in[4], -in[5]};
	l.normalize();
	l = cross(r, l);
	for (size_t i{}; i < 3; ++i)
		out[1][i] = l[i];
	l = cross(r, l);
	for (size_t i{}; i < 3; ++i)
		out[2][i] = l[i];
}

// сферическая АСК -> ортогональная АСК

void transform<abs_cs, sph_cs, abs_cs, ort_cs>::forward(double const in[3], double out[3])
{
	sph_to_ort(in, out);
}

void transform<abs_cs, sph_cs, abs_cs, ort_cs>::backward(double const in[3], double out[3])
{
	ort_to_sph(in, out);
}

// сферическая ГСК -> ортогональная ГСК

void transform<grw_cs, sph_cs, grw_cs, ort_cs>::forward(double const in[3], double out[3])
{
	sph_to_ort(in, out);
}

void transform<grw_cs, sph_cs, grw_cs, ort_cs>::backward(double const in[3], double out[3])
{
	ort_to_sph(in, out);
}

// ортогональная АСК -> ортогональная ГСК

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(double const in[3], double t, double out[3])
{
	abs_to_grw_ort(in, t, out);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(double const in[3], double t, double out[3])
{
	grw_to_abs_ort(in, t, out);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(double const inc[3], double const inv[3], double t, double w, double outc[3], double outv[3])
{
	abs_to_grw_ort(inc, inv, t, w, outc, outv);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(double const inc[3], double const inv[3], double t, double w, double outc[3], double outv[3])
{
	abs_to_grw_ort(inc, inv, t, w, outc, outv);
}

// сферическая АСК -> сферическая ГСК

void transform<abs_cs, sph_cs, grw_cs, sph_cs>::forward(double const in[3], double t, double out[3])
{
	abs_to_grw_sph(in, t, out);
}

void transform<abs_cs, sph_cs, grw_cs, sph_cs>::backward(double const in[3], double t, double out[3])
{
	grw_to_abs_sph(in, t, out);
}

// сферическая АСК -> ортогональная ГСК

void transform<abs_cs, sph_cs, grw_cs, ort_cs>::forward(double const in[3], double t, double out[3])
{
	double buf[3];
	abs_to_grw_sph(in, t, buf);
	sph_to_ort(buf, out);
}

void transform<abs_cs, sph_cs, grw_cs, ort_cs>::backward(double const in[3], double t, double out[3])
{
	double buf[3];
	grw_to_abs_ort(in, t, buf);
	ort_to_sph(buf, out);
}

// ортогональная АСК -> сферическая ГСК

void transform<abs_cs, ort_cs, grw_cs, sph_cs>::forward(const double *const in, double t, double *const out)
{
	double buf[3];
	abs_to_grw_ort(in, t, buf);
	ort_to_sph(buf, out);
}

void transform<abs_cs, ort_cs, grw_cs, sph_cs>::backward(const double *const in, double t, double *const out)
{
	double buf[3];
	grw_to_abs_sph(in, t, buf);
	sph_to_ort(buf, out);
}

// АСК -> эклиптическая

void transform<abs_cs, ort_cs, ecl_cs, sph_cs>::forward(const double *const in, double e, double *const out)
{
	double buf[3];
	abs_to_ecl(in, e, buf);
	ort_to_sph(buf, out);
}

void transform<abs_cs, ort_cs, ecl_cs, sph_cs>::backward(const double *const in, double e, double *const out)
{
	double buf[3];
	sph_to_ort(in, buf);
	ecl_to_abs(buf, e, out);
}
