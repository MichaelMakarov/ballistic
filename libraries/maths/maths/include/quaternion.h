#pragma once
#include <static_vector.h>

namespace math {

	/// <summary>
	/// Quaternion (s, x, y, z).
	/// </summary>
	struct quaternion {
		double s;	// scalar part
		vec3 v;		// vector (x, y, z)

		quaternion& operator += (const quaternion& other);
		quaternion& operator -= (const quaternion& other);
		quaternion& operator *= (const quaternion& other);
		quaternion& operator *= (double n);
		quaternion& operator /= (double n);
	};

	constexpr quaternion operator+ (const quaternion& left, const quaternion& right) {
		return quaternion{ left.s + right.s, left.v + right.v };
	}
	constexpr quaternion operator- (const quaternion& left, const quaternion& right) {
		return quaternion{ left.s - right.s, left.v - right.v };
	}
	constexpr quaternion operator*(const quaternion& q, double n) {
		return quaternion{ q.s * n, q.v * n };
	}
	constexpr quaternion operator/(const quaternion& q, double n) {
		return quaternion{ q.s / n, q.v / n };
	}
	constexpr quaternion operator*(double n, const quaternion& q) {
		return q * n;
	}

	std::ostream& operator<< (std::ostream& os, const quaternion& q);
	std::istream& operator>> (std::istream& is, quaternion& q);

	/// <summary>
	/// Computes module of quaternion.
	/// </summary>
	/// <returns></returns>
	double mod(const quaternion& q);

	/// <summary>
	/// Computes conjucted quaternion.
	/// </summary>
	/// <param name="other">is initial quaternion</param>
	/// <returns>a quaternion</returns>
	constexpr quaternion conj(const quaternion& other) {
		return quaternion{ other.s, -other.v };
	}
	/// <summary>
	/// Computes scalar multiplication of two quaternions.
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <returns>a value as a result</returns>
	constexpr double dot(const quaternion& left, const quaternion& right) {
		return left.s * right.s + left.v * right.v;
	}

	/// <summary>
	/// Computes angle between two quaternions.
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <returns>radians</returns>
	double angle(const quaternion& left, const quaternion& right);

	/// <summary>
	/// Computes inverted quaternion q^-1 which satisfies equation: q * q^-1 = 1.
	/// </summary>
	/// <param name="q"></param>
	/// <returns>a quaternion</returns>
	constexpr quaternion inverse(const quaternion& q) {
		return conj(q) / (q.s * q.s + q.v * q.v);
	}
	/// <summary>
	/// Создание кватерниона.
	/// </summary>
	/// <param name="axis">ось вращения</param>
	/// <param name="angle">угол вращения в рад</param>
	/// <returns>кватернион</returns>
	quaternion make_quaternion(const vec3 axis, double angle);
	/// <summary>
	/// Определение оси и угла вращения из кватерниона.
	/// </summary>
	/// <param name="q">кватернион</param>
	/// <returns>ось и угол</returns>
	std::pair<vec3, double> from_quaternion(const quaternion& q);
	/// <summary>
	/// Creates a quaternion of rotation between two vectors.
	/// </summary>
	quaternion rotation(vec3 left, vec3 right);
}