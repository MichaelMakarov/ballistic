#include "linalg.h"
#include "arithmetics.h"
#include "mathconstants.h"
#include <cmath>

namespace math {

    vec3 rotate_vector(const vec3& v, const vec3& axis, double angle) {
        angle *= 0.5;
        return rotate_vector(v, quaternion{ std::cos(angle), std::sin(angle) * axis });
    }

    mat3x3 make_transform(const vec3& v) noexcept {
        auto e1 = v;
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
            { e1[0], e1[1], e1[2] },
            { e2[0], e2[1], e2[2] },
            { e3[0], e3[1], e3[2] }
        };
    }
}