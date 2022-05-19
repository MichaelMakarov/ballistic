#include <linalg.h>
#include <arithmetics.h>
#include <cmath>

mat3x3 make_transform(vec3 v)
{
	auto& e1 = v;
	normalize(e1);
	size_t imax{}, imin{};
	double minval{ std::fabs(e1[0]) }, maxval{ minval }, buf;
	for (size_t i{ 1 }; i < e1.size(); ++i) {
		buf = std::fabs(e1[i]);
		if (buf > maxval) {
			maxval = buf;
			imax = i;
		}
		else if (buf < minval) {
			minval = buf;
			imin = i;
		}
	}
	maxval = std::sqrt(e1[imax] * e1[imax] + e1[imin] * e1[imin]);
	vec3 e2{};
	e2[imax] = -e1[imin] / maxval;
	e2[imin] = e1[imax] / maxval;
	auto e3 = cross(e1, e2);
	return mat3x3{
		e1[0], e1[1], e1[2],
		e2[0], e2[1], e2[2],
		e3[0], e3[1], e3[2]
	};
}

quaternion make_quaternion(vec3 axis, double angle)
{
	angle *= 0.5;
	normalize(axis);
	double sina = std::sin(angle);
	return {
		std::cos(angle),
		axis[0] * sina, axis[1] * sina, axis[2] * sina
	};
}

rotation_desc from_quaternion(const quaternion& q)
{
	double sina = std::sqrt(sqr(q[1]) + sqr(q[2]) + sqr(q[3]));
	return { vec3{ q[1], q[2], q[3] } / sina, std::atan2(sina, q[0]) * 2 };
}

quaternion rotation(vec3 left, vec3 right)
{
	normalize(left);
	normalize(right);
	return make_quaternion(
		cross(left, right),
		std::acos(left * right)
	);
}
