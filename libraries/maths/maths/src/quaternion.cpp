#include <quaternion.h>
#include <linalg.h>
#include <cmath>

namespace math {

	quaternion& quaternion::operator+=(const quaternion& other) {
		s += other.s;
		v += other.v;
		return *this;
	}

	quaternion& quaternion::operator-=(const quaternion& other) {
		s -= other.s;
		v -= other.v;
		return *this;
	}

	quaternion& quaternion::operator*=(const quaternion& other) {
		const double d = s;
		s = d * other.s - v * other.v;
		v = s * other.v + other.s * v + cross(v, other.v);
		return *this;
	}

	quaternion& quaternion::operator*=(double n) {
		s *= n;
		v *= n;
		return *this;
	}

	quaternion& quaternion::operator/=(double n) {
		s /= n;
		v /= n;
		return *this;
	}

	double angle(const quaternion& left, const quaternion& right) {
		return dot(left, right) / mod(left) / mod(right);
	}

	std::ostream& operator<<(std::ostream& os, const quaternion& q) {
		os << "( " << q.s << " " << q.v[0] << " " << q.v[1] << " " << q.v[2] << " )";
		return os;
	}

	std::istream& operator>>(std::istream& is, quaternion& q) {
		is >> q.s >> q.v[0] >> q.v[1] >> q.v[2];
		return is;
	}

	double mod(const quaternion& q)	{
		return std::sqrt(q.s * q.s + q.v * q.v);
	}

	quaternion make_quaternion(const vec3 axis, double angle)
	{
		angle *= 0.5;
		vec3 norm{ axis };
		normalize(norm);
		return quaternion{
			std::cos(angle),
			norm * std::sin(angle)
		};
	}

	std::pair<vec3, double> from_quaternion(const quaternion& q)
	{
		double sin_angle = length(q.v);
		return { q.v / sin_angle, std::atan2(sin_angle, q.s) * 2 };
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
}
