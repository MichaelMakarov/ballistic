#include <maths.hpp>

constexpr auto pi2 = 2 * std::numbers::pi;

template <>
double fit_to_round<round_type::zero_double_pi>(double radians)
{
    if (radians > pi2 || radians < 0)
    {
        radians -= std::floor(radians / pi2) * pi2;
    }
    return radians;
}

template <>
double fit_to_round<round_type::minus_plus_pi>(double radians)
{
    radians = fit_to_round<round_type::zero_double_pi>(radians);
    if (radians > std::numbers::pi)
        radians -= pi2;
    return radians;
}

mat3x3 make_transform(vec3 e1)
{
    e1.normalize();
    size_t imax{}, imin{};
    double minval{std::fabs(e1[0])}, maxval{minval}, buf;
    for (size_t i{1}; i < e1.size(); ++i)
    {
        buf = std::fabs(e1[i]);
        if (buf > maxval)
        {
            maxval = buf;
            imax = i;
        }
        else if (buf < minval)
        {
            minval = buf;
            imin = i;
        }
    }
    maxval = std::sqrt(e1[imax] * e1[imax] + e1[imin] * e1[imin]);
    vec3 e2{};
    e2[imax] = -e1[imin] / maxval;
    e2[imin] = e1[imax] / maxval;
    auto e3 = cross(e1, e2);
    mat3x3 mx;
    for (size_t i{}; i < 3; ++i)
        mx[0][i] = e1[i];
    for (size_t i{}; i < 3; ++i)
        mx[1][i] = e2[i];
    for (size_t i{}; i < 3; ++i)
        mx[2][i] = e3[i];
    return mx;
}

quaternion::quaternion(vec3 const &axis, double angle)
{
    angle *= 0.5;
    _v = axis * std::sin(angle);
    _s = std::cos(angle);
}

quaternion::quaternion(vec3 const &left, vec3 const &right) : quaternion(cross(left, right), std::acos(cos_angle_of(left, right)))
{
}

double quaternion::norm() const
{
    double norm = sqr(_s);
    for (size_t i{}; i < _v.size(); ++i)
        norm += sqr(_v[i]);
    return std::sqrt(norm);
}

void quaternion::normalize()
{
    auto mult = 1 / norm();
    _s *= mult;
    _v *= mult;
}

constexpr double cross(const vec2 left, const vec2 right)
{
    return left[0] * right[1] - left[1] * right[0];
}

double distance(const line2d &line, const vec2 &point)
{
    return cross(line.direction, point - line.point) / line.direction.length();
}

vec2 projection(const line2d &line, const vec2 &point)
{
    const auto &v = line.direction;
    double mult = cross(v, point - line.point) / (v * v);
    return vec2{point[0] + mult * v[1], point[1] - mult * v[0]};
}
