#pragma once
#include <quaternion.h>
#include <static_matrix.h>
#include <dynamic_matrix.h>
#include <polynomial.h>
#include <assertion.h>
#include <arithmetics.h>

namespace math {
    /// <summary>
    /// Computes a length of the vector.
    /// </summary>
    template<size_t size>
    double length(const vec<size>& v) { return std::sqrt(v * v); }
    /// <summary>
    /// Computes a normalized vector.
    /// </summary>
    template<size_t size>
    void normalize(vec<size>& v) { v /= length(v); }
    /// <summary>
    /// Computes vectorized multiplication.
    /// </summary>
    constexpr vec3 cross(const vec3& left, const vec3& right) noexcept {
        return vec3{
            left[1] * right[2] - left[2] * right[1],
            left[2] * right[0] - left[0] * right[2],
            left[0] * right[1] - left[1] * right[0]
        };
    }
    /// <summary>
    /// Computes multiplication of two quternions.
    /// </summary>
    constexpr quaternion operator* (const quaternion& left, const quaternion& right) {
        return quaternion{
            left.s * right.s - left.v * right.v,
            left.s * right.v + right.s * left.v + cross(left.v, right.v)
        };
    }
    /// <summary>
    /// Transforms vector using quaternion.
    /// </summary>
    /// <param name="vec">initial vector</param>
    /// <param name="qrot">quaternion that specifies rotation</param>
    /// <returns>transformed vector</returns>
    constexpr vec3 rotate_vector(const vec3& vec, const quaternion& qrot) {
        return (qrot * quaternion{ 0, vec } * inverse(qrot)).v;
    }
    /// <summary>
    /// Transforms vector by rotating around axis by angle.
    /// </summary>
    /// <param name="vec">initial vector</param>
    /// <param name="axis">axis to rotate the vector around</param>
    /// <param name="angle">angle of rotation</param>
    /// <returns>transformed vector</returns>
    vec3 rotate_vector(const vec3& vec, const vec3& axis, double angle);
    /// <summary>
    /// Computes matrix of conversion from current coordinate system to another one
    /// where specified vector is one of the basic vectors.
    /// </summary>
    /// <param name="vec">- vector that specifies direction of the first basic vector</param>
    /// <returns>matrix of transform</returns>
    mat3x3 make_transform(const vec3& vec) noexcept;

    /// <summary>
    /// Computes a length of the vector.
    /// </summary>
    double length(const vector& v);
    /// <summary>
    /// Computes a normalized vector.
    /// </summary>
    void normalize(vector& v);
    /// <summary>
    /// Computes a transposed matrix.
    /// </summary>
    matrix transpose(const matrix& m);
    /// <summary>
    /// Computes an inverted matrix and determinant.
    /// </summary>
    void inverse(matrix& mx, double* det = nullptr);
    /// <summary>
    /// Computes LU factorization (A = L * U).
    /// </summary>
    /// <param name="mx">initial matrix</param>
    /// <param name="lower">lower triangle matrix</param>
    /// <param name="upper">upper triangle matrix</param>
    void lu(const matrix& mx, matrix& lower, matrix& upper);
    /// <summary>
    /// Computes QR factorization (A = Q * R).
    /// </summary>
    /// <param name="mx">initial matrix</param>
    /// <param name="orth">orthogonal matrix</param>
    /// <param name="upper">upper triangle matrix</param>
    void qr(const matrix& mx, matrix& orth, matrix& upper);
    /// <summary>
    /// Computes singular value decomposition (A = U * S * V^T).
    /// </summary>
    /// <param name="mx">initial matrix</param>
    /// <param name="u_mx">left orthogonal matrix</param>
    /// <param name="s_mx">diagonal matrix of singular values</param>
    /// <param name="v_mx">right orthogonal matrix</param>
    void svd(const matrix& mx, matrix& u_mx, matrix& s_mx, matrix& v_mx);
    /// <summary>
    /// Computes multiplication of matrices when right matrix is diagonal.
    /// </summary>
    void mxd(matrix& mx, const vector& diag);
    /// <summary>
    /// Computes multiplication of matrices when left matrix is diagonal.
    /// </summary>
    void dxm(const vector& diag, matrix& mx);

    /// <summary>
    /// Solves system of linear equations as least squares task.
    /// </summary>
    /// <param name="mx">matrix of dimension nxm</param>
    /// <param name="vc">vector of size m</param>
    /// <returns>vector of size n</returns>
    vector lstsq(const matrix& mx, const vector& vc);

    /// <summary>
    /// Computes polynomial that approximates a list of points.
    /// </summary>
    /// <param name="x">x coordinate begin iterator</param>
    /// <param name="y">y coordinate begin iterator</param>
    /// <param name="count">a count of points</param>
    /// <returns>a polynomial of specified degree</returns>
    template<
        size_t degree, typename forw_it,
        typename O = std::enable_if<std::is_same_v<typename std::iterator_traits<forw_it>::value_type, double>>::type>
    polynomial<degree> polyfit(forw_it x, forw_it y, size_t count)
    {
        polynomial<degree> poly;
        matrix m(degree + 1, count);
        vector b(count);
        for (size_t i{}; i < count; ++i, ++x, ++y) {
            m(0, i) = 1.0;
            for (size_t k{ 1 }; k < degree + 1; ++k) m(k, i) = m(k - 1, i) * (*x);
            b[i] = *y;
        }
        auto result = lstsq(m, b);
        std::copy(std::begin(result), std::end(result), poly.data());
        return poly;
    }
    /// <summary>
    /// Computes polynomial that approximates a list of points.
    /// </summary>
    /// <param name="begin">point begin iterator</param>
    /// <param name="end">point end iterator</param>
    /// <returns>a polynomial of specified degree</returns>
    template<
        size_t degree, typename forw_it,
        typename O = std::enable_if<std::is_same_v<typename std::iterator_traits<forw_it>::value_type, vec2>>::type>
    polynomial<degree> polyfit(forw_it begin, forw_it end)
    {
        size_t count = std::distance(begin, end);
        polynomial<degree> poly;
        matrix m(degree + 1, count);
        vector b(count);
        for (size_t i{}; begin != end; ++begin, ++i) {
            const vec2& point = *begin;
            m(0, i) = 1.0;
            for (size_t k{ 1 }; k < degree + 1; ++k) m(k, i) = m(k - 1, i) * point[0];
            b[i] = point[1];
        }
        auto result = lstsq(m, b);
        std::copy(std::begin(result), std::end(result), poly.data());
        return poly;
    }

    /// <summary>
    /// Computes transposed matrix.
    /// </summary>
    template<size_t rows, size_t cols>
    constexpr mat<cols, rows> transpose(const mat<rows, cols>& mx) {
        mat<cols, rows> tr;
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c)
                tr(c, r) = mx(r, c);
        }
        return tr;
    }

    /// <summary>
    /// Computes an inverted matrix.
    /// </summary>
    template<size_t dim>
    constexpr mat<dim, dim> inverse(const mat<dim, dim>& mx) {
        auto inv{ mx };
        double pivot{};// , det{ 1 };
        size_t descent[dim];
        size_t k{};
        for (size_t r = 0; r < dim; ++r) descent[r] = r;
        for (size_t i = 0; i < dim; ++i) {
            k = descent[i];
            pivot = inv(k, k);
            //det *= pivot;
            if (math::abs(pivot) < 1e-20) {
                ASSERT(i < dim - 1, "degenerate matrix can not be inverted");
                std::swap(descent[i], descent[i + 1]);
                k = descent[i];
                pivot = inv(k, k);
            }
            for (size_t r = 0; r < dim; ++r) inv(r, k) /= -pivot;
            for (size_t r = 0; r < dim; ++r) {
                if (r != k) {
                    for (size_t c = 0; c < dim; ++c) {
                        if (c != k)
                            inv(r, c) += inv(k, c) * inv(r, k);
                    }
                }
            }
            for (size_t c = 0; c < dim; ++c) inv(k, c) /= pivot;
            inv(k, k) = 1 / pivot;
        }
        return inv;
    }
}