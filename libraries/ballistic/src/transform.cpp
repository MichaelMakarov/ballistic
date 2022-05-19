#include <transform.h>
#include <linalg.h>
#include <arithmetics.h>
#include <conversion.h>


void ort_to_sph(const double* const in, double* const out)
{
	double len = std::sqrt(sqr(in[0]) + sqr(in[1]));
	out[0] = std::sqrt(sqr(len) + sqr(in[2]));
	out[1] = std::atan2(in[2], len);
	out[2] = std::atan2(in[1], in[0]);
}

void sph_to_ort(const double* const in, double* const out)
{
	double cosl = std::cos(in[1]);
	out[0] = in[0] * std::cos(in[2]) * cosl;
	out[1] = in[0] * std::sin(in[2]) * cosl;
	out[2] = in[0] * std::sin(in[1]);
}

void abs_to_grw_ort(const double* const in, double t, double* const out)
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost + in[1] * sint;
	out[1] = in[1] * cost - in[0] * sint;
	out[2] = in[2];
}

void grw_to_abs_ort(const double* const in, double t, double* const out)
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost - in[1] * sint;
	out[1] = in[1] * cost + in[0] * sint;
	out[2] = in[2];
}

void grw_to_abs_sph(const double* const in, double t, double* const out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = fit_to_round<round_type::zero_double_pi>(in[2] - t);
}

void abs_to_grw_sph(const double* const in, double t, double* const out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = fit_to_round<round_type::zero_double_pi>(in[2] + t);
}

void ecl_to_abs(const double* const in, double e, double* const out)
{
	double sine = std::sin(e), cose = std::cos(e);
	out[0] = in[0];
	out[1] = in[1] * cose - in[2] * sine;
	out[2] = in[1] * sine + in[2] * cose;
}

void abs_to_ecl(const double* const in, double e, double* const out)
{
	double sine = std::sin(e), cose = std::cos(e);
	out[0] = in[0];
	out[1] = in[1] * cose + in[2] * sine;
	out[2] = in[1] * sine - in[2] * cose;
}

void abs_to_grw_ort(const double* const in, double t, double w, double* const out)
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost + in[1] * sint;
	out[1] = in[1] * cost - in[0] * sint;
	out[2] = in[2];
	out[3] = in[3] * cost + in[4] * sint;
	out[4] = in[4] * cost - in[3] * sint;
	out[5] = in[5];
	out[3] += w * out[1];
	out[4] -= w * out[0];
}

void grw_to_abs_ort(const double* const in, double t, double w, double* const out)
{
	double sint = std::sin(t), cost = std::cos(t);
	out[0] = in[0] * cost - in[1] * sint;
	out[1] = in[1] * cost + in[0] * sint;
	out[2] = in[2];
	out[3] = in[3] * cost - in[4] * sint;
	out[4] = in[4] * cost + in[3] * sint;
	out[5] = in[5];
	out[3] -= w * out[1];
	out[4] += w * out[0];
}

void GCS_to_OCS(const double* const in, double** const out)
{
	vec3 r{ in[0], in[1], in[2] };
	normalize(r);
	for (size_t i{}; i < 3; ++i) out[0][i] = r[i];
	vec3 l{ -in[3], -in[4], -in[5] };
	normalize(l);
	l = cross(r, l);
	for (size_t i{}; i < 3; ++i) out[1][i] = l[i];
	l = cross(r, l);
	for (size_t i{}; i < 3; ++i) out[2][i] = l[i];
}

// сферическая АСК -> ортогональная АСК

void transform<abs_cs, sph_cs, abs_cs, ort_cs>::forward(const double* const in, double* const out)
{
	sph_to_ort(in, out);
}

void transform<abs_cs, sph_cs, abs_cs, ort_cs>::backward(const double* const in, double* const out)
{
	ort_to_sph(in, out);
}

// сферическая ГСК -> ортогональная ГСК

void transform<grw_cs, sph_cs, grw_cs, ort_cs>::forward(const double* const in, double* const out)
{
	sph_to_ort(in, out);
}

void transform<grw_cs, sph_cs, grw_cs, ort_cs>::backward(const double* const in, double* const out)
{
	ort_to_sph(in, out);
}

// ортогональная АСК -> ортогональная ГСК

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(const double* const in, double t, double* const out)
{
	abs_to_grw_ort(in, t, out);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(const double* const in, double t, double* const out)
{
	grw_to_abs_ort(in, t, out);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::forward(const double* const in, double t, double w, double* const out)
{
	abs_to_grw_ort(in, t, w, out);
}

void transform<abs_cs, ort_cs, grw_cs, ort_cs>::backward(const double* const in, double t, double w, double* const out)
{
	grw_to_abs_ort(in, t, w, out);
}

// сферическая АСК -> сферическая ГСК

void transform<abs_cs, sph_cs, grw_cs, sph_cs>::forward(const double* const in, double t, double* const out)
{
	abs_to_grw_sph(in, t, out);
}

void transform<abs_cs, sph_cs, grw_cs, sph_cs>::backward(const double* const in, double t, double* const out)
{
	grw_to_abs_sph(in, t, out);
}

// сферическая АСК -> ортогональная ГСК

void transform<abs_cs, sph_cs, grw_cs, ort_cs>::forward(const double* const in, double t, double* const out)
{
	double buf[3];
	abs_to_grw_sph(in, t, buf);
	sph_to_ort(buf, out);
}

void transform<abs_cs, sph_cs, grw_cs, ort_cs>::backward(const double* const in, double t, double* const out)
{
	double buf[3];
	grw_to_abs_ort(in, t, buf);
	ort_to_sph(buf, out);
}

// ортогональная АСК -> сферическая ГСК

void transform<abs_cs, ort_cs, grw_cs, sph_cs>::forward(const double* const in, double t, double* const out)
{
	double buf[3];
	abs_to_grw_ort(in, t, buf);
	ort_to_sph(buf, out);
}

void transform<abs_cs, ort_cs, grw_cs, sph_cs>::backward(const double* const in, double t, double* const out)
{
	double buf[3];
	grw_to_abs_sph(in, t, buf);
	sph_to_ort(buf, out);
}

// АСК -> эклиптическая

void transform<abs_cs, sph_cs, ecl_cs, sph_cs>::forward(const double* const in, double e, double* const out)
{
	abs_to_ecl(in, e, out);
}

void transform<abs_cs, sph_cs, ecl_cs, sph_cs>::backward(const double* const in, double e, double* const out)
{
	ecl_to_abs(in, e, out);
}
