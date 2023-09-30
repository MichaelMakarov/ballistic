#include <maths.hpp>
#include <limits>
#include <vector>
#include <cstring>
#include <cmath>
#include <iostream>
#include <stdexcept>
#ifdef __AVX__
#include <immintrin.h>
#if defined WIN32
#include <intrin.h>
#endif
#endif

namespace math
{

    void throw_invalid_argument(char const *msg)
    {
        throw std::invalid_argument(msg);
    }

    void throw_runtime_error(char const *msg)
    {
        throw std::runtime_error(msg);
    }

    void throw_out_of_range(char const *msg)
    {
        throw std::out_of_range(msg);
    }

    constexpr auto pi2{pi * 2};
    constexpr double zero = std::numeric_limits<double>::epsilon();
    constexpr size_t allign{256 / 8 / sizeof(double)};

    template <>
    double fit_round<round_type::zdpi>(double radians)
    {
        return radians - std::floor(radians / pi2) * pi2;
    }

    template <>
    double fit_round<round_type::mppi>(double radians)
    {
        radians = fit_round<round_type::zdpi>(radians);
        if (radians > pi)
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
        auto &q = *this;
        return std::sqrt(dot(q, q));
    }

    void quaternion::normalize()
    {
        double mult = 1 / norm();
        _s *= mult;
        _v *= mult;
    }

    quaternion quaternion::from_plane_angles(double kren, double tang, double risk)
    {
        kren *= 0.5;
        tang *= 0.5;
        risk *= 0.5;
        double cosk = std::cos(kren), sink = std::sin(kren);
        double cost = std::cos(tang), sint = std::sin(tang);
        double cosr = std::cos(risk), sinr = std::sin(risk);
        return quaternion{
            sink * cost * cosr + cosk * sint * sinr,
            sink * cost * sinr + cosk * sint * cosr,
            cosk * cost * sinr - sink * sint * cosr,
            cosk * cost * cosr - sink * sint * sinr};
    }

    void quaternion::to_plane_angles(quaternion const &q, double &kren, double &tang, double &risk)
    {
        double s = q._s, x = q._v[0], y = q._v[1], z = q._v[2];
        double s2 = sqr(s), x2 = sqr(x), y2 = sqr(y), z2 = sqr(z);
        kren = std::atan2(-2 * (y * z - s * x), s2 - x2 + y2 - z2);
        tang = std::atan2(-2 * (x * z - s * y), s2 + x2 - y2 - z2);
        risk = std::asin(2 * (s * z + x * y));
    }

    void quaternion::cos_matrix_to_euler(double *matrix, double &risk, double &tang, double &kren, RotationOrder rotation)
    {
        // функция переписана с матлаба
        risk = 0;
        tang = 0;
        kren = 0;
        switch (rotation)
        {
        // X-крен, Y - тангаж, Z - рысканье
        case zyx:
        default:
            //    %     [          cy*cz,          cy*sz,            -sy]
            //    %     [ sy*sx*cz-sz*cx, sy*sx*sz+cz*cx,          cy*sx]
            //    %     [ sy*cx*cz+sz*sx, sy*cx*sz-cz*sx,          cy*cx]
            risk = atan2(matrix[1], matrix[0]);
            tang = asin(-matrix[2]);
            kren = atan2(matrix[5], matrix[8]);
            break;
        case zyz:
            //    %     [  cz2*cy*cz-sz2*sz,  cz2*cy*sz+sz2*cz,           -cz2*sy]
            //    %     [ -sz2*cy*cz-cz2*sz, -sz2*cy*sz+cz2*cz,            sz2*sy]
            //    %     [             sy*cz,             sy*sz,                cy]
            risk = atan2(matrix[7], matrix[6]);
            tang = acos(matrix[8]);
            kren = atan2(matrix[5], -matrix[2]);
            break;
        case zxy:
            //    %     [ cy*cz-sy*sx*sz, cy*sz+sy*sx*cz,         -sy*cx]
            //    %     [         -sz*cx,          cz*cx,             sx]
            //    %     [ sy*cz+cy*sx*sz, sy*sz-cy*sx*cz,          cy*cx]
            risk = atan2(matrix[4], -matrix[3]);
            tang = atan2(-matrix[2], matrix[8]);
            kren = asin(matrix[5]);
            break;
        case zxz:
            //    %     [  cz2*cz-sz2*cx*sz,  cz2*sz+sz2*cx*cz,            sz2*sx]
            //    %     [ -sz2*cz-cz2*cx*sz, -sz2*sz+cz2*cx*cz,            cz2*sx]
            //    %     [             sz*sx,            -cz*sx,                cx]
            risk = atan2(matrix[6], -matrix[7]);
            tang = atan2(matrix[2], matrix[5]);
            kren = acos(matrix[8]);
            break;
        case yxz:
            //    %     [  cy*cz+sy*sx*sz,           sz*cx, -sy*cz+cy*sx*sz]
            //    %     [ -cy*sz+sy*sx*cz,           cz*cx,  sy*sz+cy*sx*cz]
            //    %     [           sy*cx,             -sx,           cy*cx]
            risk = atan2(matrix[1], matrix[4]);
            tang = atan2(matrix[6], matrix[8]);
            kren = asin(-matrix[7]);
            break;
        case yzy:
            //    %     [  cy2*cz*cy-sy2*sy,            cy2*sz, -cy2*cz*sy-sy2*cy]
            //    %     [            -cy*sz,                cz,             sy*sz]
            //    %     [  sy2*cz*cy+cy2*sy,            sy2*sz, -sy2*cz*sy+cy2*cy]
            risk = acos(matrix[4]);
            tang = atan2(matrix[5], -matrix[3]);
            kren = atan2(matrix[7], matrix[1]);
            break;
        case xyz:
            //    %     [          cy*cz, sz*cx+sy*sx*cz, sz*sx-sy*cx*cz]
            //    %     [         -cy*sz, cz*cx-sy*sx*sz, cz*sx+sy*cx*sz]
            //    %     [             sy,         -cy*sx,          cy*cx]
            risk = atan2(-matrix[3], matrix[0]);
            tang = asin(matrix[6]);
            kren = atan2(-matrix[7], matrix[8]);
            break;
        case xyx:
            //    %     [                cy,             sy*sx,            -sy*cx]
            //    %     [            sx2*sy,  cx2*cx-sx2*cy*sx,  cx2*sx+sx2*cy*cx]
            //    %     [            cx2*sy, -sx2*cx-cx2*cy*sx, -sx2*sx+cx2*cy*cx]
            risk = atan2(matrix[3], matrix[6]);
            tang = acos(matrix[0]);
            kren = atan2(matrix[1], -matrix[2]);
            break;
        case xzy:
            //    %     [          cy*cz, sz*cx*cy+sy*sx, cy*sx*sz-sy*cx]
            //    %     [            -sz,          cz*cx,          cz*sx]
            //    %     [          sy*cz, sy*cx*sz-cy*sx, sy*sx*sz+cy*cx]
            risk = asin(-matrix[3]);
            tang = atan2(matrix[6], matrix[0]);
            kren = atan2(matrix[5], matrix[4]);
            break;
        case xzx:
            //    %     [                cz,             sz*cx,             sz*sx]
            //    %     [           -cx2*sz,  cx2*cz*cx-sx2*sx,  cx2*cz*sx+sx2*cx]
            //    %     [            sx2*sz, -sx2*cz*cx-cx2*sx, -sx2*cz*sx+cx2*cx]
            risk = acos(matrix[0]);
            tang = atan2(matrix[6], -matrix[3]);
            kren = atan2(matrix[2], matrix[1]);
            break;
        }
        risk = atan2(matrix[1], matrix[0]);
        tang = asin(-matrix[2]);
        kren = atan2(matrix[5], matrix[8]);
    }

    void quaternion::angles_to_quat(double risk, double tang, double kren, quaternion &quat, RotationOrder rotation)
    {
        double halfAng1 = risk * 0.5;
        double halfAng2 = tang * 0.5;
        double halfAng3 = kren * 0.5;
        switch (rotation)
        {
        case RotationOrder::zyx:
        default:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[2] = sin(halfAng1) * cos(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            break;
        case zyz:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[2] = sin(halfAng1) * cos(halfAng2) * cos(halfAng3) + cos(halfAng1) * cos(halfAng2) * sin(halfAng3);
            break;
        case zxy:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[1] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            break;
        case zxz:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[1] = sin(halfAng1) * sin(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[2] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            break;
        case yxz:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[1] = sin(halfAng1) * cos(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[2] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            break;
        case yxy:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[1] = sin(halfAng1) * cos(halfAng2) * cos(halfAng3) + cos(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            break;
        case yzx:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            break;
        case yzy:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = sin(halfAng1) * sin(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[1] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            break;
        case xyz:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[2] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            break;
        case xyx:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[2] = sin(halfAng1) * sin(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            break;
        case xzy:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[0] = sin(halfAng1) * cos(halfAng2) * cos(halfAng3) - cos(halfAng1) * sin(halfAng2) * sin(halfAng3);
            quat._v[1] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            break;
        case xzx:
            quat._s = cos(halfAng1) * cos(halfAng2) * cos(halfAng3) - sin(halfAng1) * cos(halfAng2) * sin(halfAng3);
            quat._v[0] = cos(halfAng1) * cos(halfAng2) * sin(halfAng3) + sin(halfAng1) * cos(halfAng2) * cos(halfAng3);
            quat._v[1] = cos(halfAng1) * sin(halfAng2) * sin(halfAng3) - sin(halfAng1) * sin(halfAng2) * cos(halfAng3);
            quat._v[2] = cos(halfAng1) * sin(halfAng2) * cos(halfAng3) + sin(halfAng1) * sin(halfAng2) * sin(halfAng3);
            break;
        }
        quat.normalize();
    }

    void quaternion::quat_to_angles(quaternion &quat, double &tang, double &kren, double &risk)
    {
        matrix _matrixCos(3, 3);
        quaternion quatNorm(quat.x(), quat.y(), quat.z(), quat.s());
        quatNorm.normalize();
        quater_to_vsk_isk(quatNorm, _matrixCos.data());
        matrix _matrixCosT = transpose(_matrixCos);
        cos_matrix_to_euler(_matrixCosT.data(), risk, tang, kren);
    }

    void quaternion::quater_to_vsk_isk(quaternion &q, double *c)
    {
        double l0 = q.s();
        double l1 = q.x();
        double l2 = q.y();
        double l3 = q.z();

        /*c[0] = l0 * l0 + l1 * l1 - l2 * l2 - l3 * l3;
        c[1] = 2.0 * (l1 * l2 + l0 * l3);
        c[2] = 2.0 * (l1 * l3 - l0 * l2);
        c[3] = 2.0 * (l1 * l2 - l0 * l3);
        c[4] = l0 * l0 - l1 * l1 + l2 * l2 - l3 * l3;
        c[5] = 2.0 * (l2 * l3 + l0 * l1);
        c[6] = 2.0 * (l1 * l3 + l0 * l2);
        c[7] = 2.0 * (l2 * l3 - l0 * l1);
        c[8] = l0 * l0 - l1 * l1 - l2 * l2 + l3 * l3;*/
        c[0] = l0 * l0 + l1 * l1 - l2 * l2 - l3 * l3;
        c[1] = 2.0 * (-l0 * l3 + l1 * l2);
        c[2] = 2.0 * (l0 * l2 + l1 * l3);
        c[3] = 2.0 * (l0 * l3 + l1 * l2);
        c[4] = l0 * l0 + l2 * l2 - l1 * l1 - l3 * l3;
        c[5] = 2.0 * (-l0 * l1 + l2 * l3);
        c[6] = 2.0 * (-l0 * l2 + l1 * l3);
        c[7] = 2.0 * (l0 * l1 + l2 * l3);
        c[8] = l0 * l0 + l3 * l3 - l1 * l1 - l2 * l2;
    }

    void quaternion::quat_from_MatrixStanley(double *matr, quaternion &quat)
    {
        double M[3][3];
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                M[i][j] = matr[3 * i + j];
            }
        }

        double T, val_s, val_x, val_y, val_z;

        try
        {
            T = M[0][0] + M[1][1] + M[2][2];

            val_s = (1 + T) / 4;
            val_x = (1 + 2 * M[0][0] - T) / 4;
            val_y = (1 + 2 * M[1][1] - T) / 4;
            val_z = (1 + 2 * M[2][2] - T) / 4;

            if ((val_s >= val_x) && (val_s >= val_y) && (val_s >= val_z))
            {
                const auto s = std::sqrt(val_s);
                quaternion q((M[1][2] - M[2][1]) / (4 * s),
                             (M[2][0] - M[0][2]) / (4 * s),
                             (M[0][1] - M[1][0]) / (4 * s),
                             s);
                quat = q;
            }

            if ((val_x >= val_s) && (val_x >= val_y) && (val_x >= val_z))
            {
                auto x = std::sqrt(val_x);
                quaternion q(x,
                             (M[0][1] + M[1][0]) / (4 * x),
                             (M[2][0] + M[0][2]) / (4 * x),
                             (M[1][2] - M[2][1]) / (4 * x));
                quat = q;
            }

            if ((val_y >= val_s) && (val_y >= val_x) && (val_y >= val_z))
            {
                auto y = std::sqrt(val_y);
                quaternion q((M[0][1] + M[1][0]) / (4 * y),
                             y,
                             (M[1][2] + M[2][1]) / (4 * y),
                             (M[2][0] - M[0][2]) / (4 * y));
                quat = q;
            }

            if ((val_z >= val_x) && (val_z >= val_y) && (val_z >= val_s))
            {
                auto z = std::sqrt(val_z);
                quaternion q((M[2][0] + M[0][2]) / (4 * z),
                             (M[1][2] + M[2][1]) / (4 * z),
                             z,
                             (M[0][1] - M[1][0]) / (4 * z));
                quat = q;
            }
            quat.normalize();
            quat = inverse(quat);
        }
        catch (std::exception const &ex)
        {
            std::cout << ex.what() << std::endl;
            quat = quaternion(0, 0, 0, 0);
        }
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

    //----------------------------------------------------------------------

#ifdef __AVX__
    static bool avxsupport()
    {
        bool support{false};
        bool cpusupport{false};
        bool ossupport{true};
#if defined WIN32
        int info[4]{};
        __cpuid(info, 1);
        ossupport = info[2] & (1 << 27) || false;
        cpusupport = info[2] & (1 << 28) || false;
        if (ossupport && cpusupport)
        {
            unsigned long long feature_mask = _xgetbv(0); // _XCR_XFEATURE_ENABLED_MASK);
            support = (feature_mask & 0x6) == 0x6;
        }
#else
        cpusupport = __builtin_cpu_supports("avx");
        //__builtin_os
        support = cpusupport & ossupport;
#endif
        return support;
    }
#endif

    void forw_add(const double *left, const double *right, double *out, size_t size)
    {
        for (auto end = out + size; out != end; ++out)
        {
            (*out) = (*left++) + (*right++);
        }
    }

    void forw_sub(const double *left, const double *right, double *out, size_t size)
    {
        for (auto end = out + size; out != end; ++out)
        {
            (*out) = (*left++) - (*right++);
        }
    }

    void forw_mul(const double *in, double mul, double *out, size_t size)
    {
        for (auto end = out + size; out != end; ++out)
        {
            (*out) = (*in++) * mul;
        }
    }

    void forw_mul(const double *left, const double *right, double *out, size_t rows, size_t mids, size_t cols)
    {
        for (size_t r{}; r < rows; ++r)
        {
            for (size_t k{}; k < mids; ++k)
            {
                for (size_t c{}; c < cols; ++c)
                {
                    out[r * cols + c] += left[r * mids + k] * right[k * cols + c];
                }
            }
        }
    }

    void forw_mul(const double *left, const double *right, double *out, size_t rows, size_t cols)
    {
        for (size_t r{}; r < rows; ++r)
        {
            for (size_t c{}; c < cols; ++c)
            {
                out[r] += left[r * cols + c] * right[c];
            }
        }
    }

    void forw_mul(const double *left, const double *right, double &out, size_t size)
    {
        for (size_t i{}; i < size; ++i)
        {
            out += left[i] * right[i];
        }
    }

#ifdef __AVX__

    void simd_add(const double *left, const double *right, double *out, size_t size)
    {
        const size_t count{size / allign};
        for (size_t i{}; i < count; ++i)
        {
            auto out_md = _mm256_add_pd(_mm256_load_pd(left), _mm256_load_pd(right));
            _mm256_store_pd(out, out_md);
            left += allign;
            right += allign;
            out += allign;
        }
        forw_add(left, right, out, size - count * allign);
    }

    void simd_sub(const double *left, const double *right, double *out, size_t size)
    {
        const size_t count{size / allign};
        for (size_t i{}; i < count; ++i)
        {
            auto out_md = _mm256_sub_pd(_mm256_load_pd(left), _mm256_load_pd(right));
            _mm256_store_pd(out, out_md);
            left += allign;
            right += allign;
            out += allign;
        }
        forw_sub(left, right, out, size - count * allign);
    }

    void simd_mul(const double *in, double mul, double *out, size_t size)
    {
        const size_t count{size / allign};
        auto mul_md = _mm256_set1_pd(mul);
        for (size_t i{}; i < count; ++i)
        {
            auto out_md = _mm256_mul_pd(_mm256_load_pd(in), mul_md);
            _mm256_store_pd(out, out_md);
            in += allign;
            out += allign;
        }
        forw_mul(in, mul, out, size - count * allign);
    }

    void simd_mul(const double *left, const double *right, double *out, size_t rows, size_t mids, size_t cols)
    {
        size_t count{cols / allign};
        for (size_t r{}; r < rows; ++r)
        {
            for (size_t k{}; k < mids; ++k)
            {
                // элемент левой матрицы
                double mul = left[r * mids + k];
                auto left_md = _mm256_set1_pd(mul);
                // указатель на строку правой матрицы
                auto right_ptr = right + k * cols;
                // указатель на строку матрицы результата
                auto out_ptr = out + r * cols;
                for (size_t c{}; c < count; ++c)
                {
                    auto out_md = _mm256_load_pd(out_ptr);
                    auto tmp_md = _mm256_mul_pd(left_md, _mm256_load_pd(right_ptr));
                    tmp_md = _mm256_add_pd(out_md, tmp_md);
                    _mm256_store_pd(out_ptr, tmp_md);
                    out_ptr += allign;
                    right_ptr += allign;
                }
                for (size_t c{count * allign}; c < cols; ++c)
                {
                    (*out_ptr++) += (*right_ptr++) * mul;
                }
            }
        }
    }

    void simd_mul(const double *left, const double *right, double *out, size_t rows, size_t cols)
    {
        size_t count{cols / allign};
        double arr[allign]{};
        for (size_t r{}; r < rows; ++r)
        {
            auto out_md = _mm256_setzero_pd();
            for (size_t c{}; c < count; ++c)
            {
                out_md = _mm256_add_pd(out_md, _mm256_mul_pd(_mm256_load_pd(left), _mm256_load_pd(right + c * allign)));
                left += allign;
            }
            _mm256_store_pd(arr, out_md);
            for (auto a : arr)
            {
                (*out) += a;
            }
            for (size_t c{count * allign}; c < cols; ++c)
            {
                (*out) += (*left++) * right[c];
            }
            ++out;
        }
    }

    void simd_mul(const double *left, const double *right, double &out, size_t size)
    {
        size_t count{size / allign};
        auto sum_md = _mm256_setzero_pd();
        for (size_t i{}; i < count; ++i)
        {
            auto mul_md = _mm256_mul_pd(_mm256_load_pd(left), _mm256_load_pd(right));
            sum_md = _mm256_add_pd(sum_md, mul_md);
            left += allign;
            right += allign;
        }
        double arr[allign]{};
        _mm256_store_pd(arr, sum_md);
        for (auto a : arr)
        {
            out += a;
        }
        forw_mul(left, right, out, size - count * allign);
    }

#endif

    void (*add_ptr)(const double *, const double *, double *, size_t);
    void (*sub_ptr)(const double *, const double *, double *, size_t);
    void (*mul_ptr)(const double *, double, double *, size_t);
    void (*mulmm_ptr)(const double *, const double *, double *, size_t, size_t, size_t);
    void (*mulvv_ptr)(const double *, const double *, double &, size_t);
    void (*mulmv_ptr)(const double *, const double *, double *, size_t, size_t);

    static bool initialize()
    {
#ifdef __AVX__
        if (avxsupport())
        {
            add_ptr = &simd_add;
            sub_ptr = &simd_sub;
            mul_ptr = &simd_mul;
            mulmm_ptr = &simd_mul;
            mulmv_ptr = &simd_mul;
            mulvv_ptr = &simd_mul;
            return true;
        }
#endif
        add_ptr = &forw_add;
        sub_ptr = &forw_sub;
        mul_ptr = &forw_mul;
        mulmm_ptr = &forw_mul;
        mulmv_ptr = &forw_mul;
        mulvv_ptr = &forw_mul;
        return false;
    }

    const bool ___ = initialize();

    //----------------------//
    //      матрица         //
    //----------------------//

    void matrix::copy(const matrix &other)
    {
        _rows = other._rows;
        _cols = other._cols;
        _elems = new double[_rows * _cols];
        std::memcpy(_elems, other._elems, _rows * _cols * sizeof(double));
    }

    void matrix::move(matrix &other) noexcept
    {
        std::swap(_rows, other._rows);
        std::swap(_cols, other._cols);
        std::swap(_elems, other._elems);
    }

    bool check_dimension(const matrix &left, const matrix &right) noexcept
    {
        return left._rows == right._rows && left._cols == right._cols;
    }

    matrix::matrix() noexcept : _elems{nullptr}, _rows{}, _cols{}
    {
    }

    matrix::matrix(const size_t rows, const size_t columns) : _elems{nullptr}, _rows{rows}, _cols{columns}
    {
        if (_rows * _cols > 0)
        {
            _elems = new double[_rows * _cols]{};
        }
    }

    size_t columns_of(const std::initializer_list<std::initializer_list<double>> &list)
    {
        size_t columns{};
        for (auto &inner : list)
        {
            columns = std::max(columns, inner.size());
        }
        return columns;
    }

    matrix::matrix(std::initializer_list<std::initializer_list<double>> list) : matrix(list.size(), columns_of(list))
    {
        size_t r{};
        for (const auto &inner : list)
        {
            size_t c{};
            for (double val : inner)
            {
                _elems[r * _cols + c] = val;
                ++c;
            }
            ++r;
        }
    }

    matrix::matrix(const matrix &other)
    {
        this->copy(other);
    }

    matrix::matrix(matrix &&other) noexcept : matrix()
    {
        move(other);
    }

    matrix::~matrix()
    {
        if (_elems)
        {
            delete[] _elems;
            _elems = nullptr;
        }
    }

    matrix &matrix::operator=(const matrix &other)
    {
        copy(other);
        return *this;
    }

    matrix &matrix::operator=(matrix &&other) noexcept
    {
        move(other);
        return *this;
    }

#define throw_on_mult throw_invalid_argument("У перемножаемых матриц кол-во столбцов левой матрицы не равно кол-ву строк правой.");

    matrix &matrix::operator+=(const matrix &other)
    {
        if (!check_dimension(*this, other))
        {
            throw_invalid_argument("Складываемая матрица имеет другую размерность.");
        }
        add_ptr(_elems, other._elems, _elems, _rows * _cols);
        return *this;
    }

    matrix &matrix::operator-=(const matrix &other)
    {
        if (!check_dimension(*this, other))
        {
            throw_invalid_argument("Вычитаемая матрица имеет другую размерность.");
        }
        sub_ptr(_elems, other._elems, _elems, _rows * _cols);
        return *this;
    }

    matrix &matrix::operator*=(double value)
    {
        mul_ptr(_elems, value, _elems, _rows * _cols);
        return *this;
    }

    matrix &matrix::operator/=(double value)
    {
        mul_ptr(_elems, 1 / value, _elems, _rows * _cols);
        return *this;
    }

    matrix operator*(const matrix &left, const matrix &right)
    {
        if (left._cols != right._rows)
        {
            throw_on_mult
        }
        matrix out(left._rows, right._cols);
        mulmm_ptr(left._elems, right._elems, out._elems, left._rows, left._cols, right._cols);
        return out;
    }

    vector operator*(const matrix &mx, const vector &vc)
    {
        if (mx._cols != vc.size())
        {
            throw_invalid_argument("Кол-во столбцов матрицы не равно размерности вектора.");
        }
        vector out(mx._rows);
        mulmv_ptr(mx._elems, vc.data(), out.data(), mx._rows, mx._cols);
        return out;
    }

    matrix operator+(const matrix &left, const matrix &right)
    {
        return matrix(left) += right;
    }

    matrix operator-(const matrix &left, const matrix &right)
    {
        return matrix(left) -= right;
    }

    matrix operator/(const matrix &mx, double value)
    {
        return matrix(mx) /= value;
    }

    matrix operator*(const matrix &mx, double value)
    {
        return matrix(mx) *= value;
    }

    matrix operator*(double value, const matrix &mx)
    {
        return matrix(mx) *= value;
    }

    matrix diag(size_t dim, double value)
    {
        matrix mx(dim, dim);
        for (size_t i{}; i < dim; ++i)
            mx[i][i] = value;
        return mx;
    }

    matrix transpose(const matrix &mx)
    {
        size_t rows = mx.rows(), cols = mx.columns();
        matrix tr(cols, rows);
        for (size_t r{}; r < rows; ++r)
        {
            for (size_t c{}; c < cols; ++c)
            {
                tr[c][r] = mx[r][c];
            }
        }
        return tr;
    }

    namespace detail
    {
        double sqrt(double val)
        {
            return std::sqrt(val);
        }

        inline double &element(double *ptr, size_t size, size_t r, size_t c)
        {
            return ptr[r * size + c];
        }

        double inverse(double *mx, size_t size)
        {
            // центральный элемент строки, относительно которой производятся преобразования
            double pivot{};
            // определитель
            double det{1};
            // массив индексов строк
            std::vector<size_t> descent(size);
            for (size_t i{}; i < size; ++i)
                descent[i] = i;
            for (size_t i{}; i < size; ++i)
            {
                size_t k = descent[i];
                pivot = element(mx, size, k, k);
                // если текущие элемент на диагонали равен нулю, то меняем на другой
                if (std::fabs(pivot) < zero)
                {
                    for (size_t n{i + 1}; n < size; ++n)
                    {
                        size_t m = descent[n];
                        double val = element(mx, size, m, m);
                        if (std::fabs(val) > zero)
                        {
                            pivot = val;
                            k = m;
                            std::swap(descent[i], descent[n]);
                            break;
                        }
                    }
                    if (std::fabs(pivot) < zero)
                        return 0;
                }
                det *= pivot;
                for (size_t r{}; r < size; ++r)
                    element(mx, size, r, k) /= -pivot;
                for (size_t r{}; r < size; ++r)
                {
                    if (r != k)
                    {
                        for (size_t c{}; c < size; ++c)
                        {
                            if (c != k)
                                element(mx, size, r, c) += element(mx, size, k, c) * element(mx, size, r, k);
                        }
                    }
                }
                for (size_t c{}; c < size; ++c)
                    element(mx, size, k, c) /= pivot;
                element(mx, size, k, k) = 1 / pivot;
            }
            return det;
        }
    }

#define throw_on_notsq throw_invalid_argument("Матрица не является квадратной.");

    double inverse(matrix &mx)
    {
        if (mx.rows() != mx.columns())
        {
            throw_on_notsq
        }
        return detail::inverse(mx.data(), mx.rows());
    }

    void lu(const matrix &mx, matrix &lower, matrix &upper)
    {
        size_t rows = mx.rows(), cols = mx.columns();
        if (rows != cols)
        {
            throw_on_notsq
        }
        upper = lower = matrix(rows, cols);
        double sum;
        for (size_t r{}; r < rows; ++r)
        {
            for (size_t c{}; c < cols; ++c)
            {
                sum = 0;
                for (size_t i{}; i < cols; ++i)
                    sum += lower[r][i] * upper[i][c];
                if (r <= c)
                {
                    upper[r][c] = mx[r][c] - sum;
                }
                else
                {
                    lower[r][c] = (mx[r][c] - sum) / upper[r][c];
                }
            }
            lower[r][r] = 1.0;
        }
    }

    void normalize(vector &v);

    void mxd(matrix &mx, const vector &diag)
    {
        size_t rows = mx.rows(), cols = mx.columns();
        if (cols != diag.size())
        {
            throw_on_mult
        }
        for (size_t r{}; r < rows; ++r)
            for (size_t c{}; c < cols; ++c)
                mx[r][c] *= diag[c];
    }

    void dxm(const vector &diag, matrix &mx)
    {
        size_t rows = mx.rows(), cols = mx.columns();
        if (cols != diag.size())
        {
            throw_on_mult
        }
        for (size_t r{}; r < rows; ++r)
            for (size_t c{}; c < cols; ++c)
                mx[r][c] *= diag[r];
    }

    //------------------//
    //      вектор      //
    //------------------//

    void vector::copy(const vector &other)
    {
        _size = other._size;
        _elems = new double[_size];
        std::memcpy(_elems, other._elems, _size * sizeof(double));
    }

    void vector::move(vector &other) noexcept
    {
        std::swap(_size, other._size);
        std::swap(_elems, other._elems);
    }

    vector::vector() noexcept
    {
        _size = 0;
        _elems = nullptr;
    }

    vector::vector(size_t size)
    {
        _size = size;
        _elems = new double[_size]{};
    }

    vector::vector(size_t size, double value)
    {
        _size = size;
        _elems = new double[_size];
        std::fill(begin(), end(), value);
    }

    vector::vector(std::initializer_list<double> list) : vector(list.size())
    {
        std::copy(std::begin(list), std::end(list), begin());
    }

    vector::vector(const vector &other)
    {
        copy(other);
    }

    vector::vector(vector &&other) noexcept : vector()
    {
        move(other);
    }

    vector::~vector()
    {
        if (_elems)
        {
            delete[] _elems;
            _elems = nullptr;
        }
    }

    vector &vector::operator=(const vector &other)
    {
        copy(other);
        return *this;
    }

    vector &vector::operator=(vector &&other) noexcept
    {
        move(other);
        return *this;
    }

    vector &vector::operator+=(const vector &other)
    {
        if (_size != other._size)
        {
            throw_invalid_argument("Добавляемый вектор имеет другую размерность.");
        }
        add_ptr(_elems, other._elems, _elems, _size);
        return *this;
    }

    vector &vector::operator-=(const vector &other)
    {
        if (_size != other._size)
        {
            throw_invalid_argument("Вычитаемый вектор имеет другую размерность.");
        }
        sub_ptr(_elems, other._elems, _elems, _size);
        return *this;
    }

    vector &vector::operator*=(double value)
    {
        mul_ptr(_elems, value, _elems, _size);
        return *this;
    }

    vector &vector::operator/=(double value)
    {
        mul_ptr(_elems, 1 / value, _elems, _size);
        return *this;
    }

    double operator*(const vector &left, const vector &right)
    {
        if (left._size != right._size)
        {
            throw_invalid_argument("Перемножаемые векторы имеют разные размерности.");
        }
        double result{};
        mulvv_ptr(left._elems, right._elems, result, left._size);
        return result;
    }

    vector operator+(const vector &left, const vector &right)
    {
        return vector(left) += right;
    }

    vector operator-(const vector &left, const vector &right)
    {
        return vector(left) -= right;
    }

    vector operator*(double value, const vector &vec)
    {
        return vector(vec) *= value;
    }

    vector operator*(const vector &vec, double value)
    {
        return vector(vec) *= value;
    }

    vector operator/(const vector &vec, double value)
    {
        return vector(vec) /= value;
    }

    double *vector::begin()
    {
        return _elems;
    }

    const double *vector::begin() const
    {
        return _elems;
    }

    double *vector::end()
    {
        return _elems + _size;
    }

    const double *vector::end() const
    {
        return _elems + _size;
    }

#define throw_if_empty throw_out_of_range("Пустой вектор не имеет элементов.");

    double &vector::first()
    {
        if (_size == 0)
        {
            throw_if_empty
        }
        return _elems[0];
    }

    const double &vector::first() const
    {
        if (_size == 0)
        {
            throw_if_empty
        }
        return _elems[0];
    }

    double &vector::last()
    {
        if (_size == 0)
        {
            throw_if_empty
        }
        return _elems[_size - 1];
    }

    const double &vector::last() const
    {
        if (_size == 0)
        {
            throw_if_empty
        }
        return _elems[_size - 1];
    }

    double vector::length() const
    {
        return std::sqrt(sqr(*this));
    }

    void vector::normalize()
    {
        (*this) /= length();
    }

    vector lstsq(const matrix &mx, const vector &vc, matrix const *cor)
    {
        auto smx = mx * transpose(mx);
        if (cor)
        {
            smx += *cor;
        }
        size_t rows = smx.rows();
        vector diag(rows);
        for (size_t i{}; i < rows; ++i)
        {
            diag[i] = 1 / std::sqrt(smx[i][i]);
        }
        mxd(smx, diag);
        dxm(diag, smx);
        if (0 == inverse(smx))
        {
            throw_runtime_error("Матрица вырождена и не может быть обращена.");
        }
        mxd(smx, diag);
        dxm(diag, smx);
        return smx * (mx * vc);
    }
}
