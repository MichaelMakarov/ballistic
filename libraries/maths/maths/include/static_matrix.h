#pragma once
#include "static_vector.h"

namespace math {

    /// матрица константной размерности
    template<size_t _rows, size_t _cols>
    class mat {
        static_assert(_rows * _cols > size_t{}, "attempt to create empty mxn");
    public:
        using value_type = double;
        using reference = value_type&;
        using const_reference = const value_type&;
    public:
        double _elems[_rows][_cols]{};
    public:
        mat() = default;
        constexpr mat(std::initializer_list<std::initializer_list<double>> lists) {
            if (lists.size() > _rows) {
                throw std::invalid_argument("initializer list values count out of rows number");
            }
            size_t r{};
            for (auto row : lists) {
                if (row.size() > _cols) {
                    throw std::invalid_argument("initializer list values count out of columns number");
                }
                size_t c{};
                for (double val : row) _elems[r][c++] = val;
                ++r;
            }
        }
        mat(const mat& other) = default;
        mat(mat&& other) noexcept = default;

        mat& operator= (const mat& other) = default;
        mat& operator= (mat&& other) noexcept = default;

        /// кол-во строк
        constexpr size_t rows() const noexcept { return _rows; }
        /// кол-во столбцов
        constexpr size_t columns() const noexcept { return _cols; }

        /// указатель на массив значений
        double** data() noexcept { return _elems; }
        /// указатель на массив значений
        const double** data() const noexcept { return _elems; }

        [[nodiscard]] constexpr reference operator() (size_t r, size_t c) {
            return _elems[r][c];
        }
        [[nodiscard]] constexpr const_reference operator() (size_t r, size_t c) const {
            return _elems[r][c];
        }

        mat& operator+= (const mat& other);
        mat& operator-= (const mat& other);
        mat& operator*= (double value);
        mat& operator/= (double value);
    };

    /// <summary>
    /// Создание диагональной матрицы.
    /// </summary>
    /// <param name="value">значение диагональных элементов</param>
    /// <returns>матрица</returns>
    template<size_t size>
    constexpr mat<size, size> diag(double value = 1.0) {
        mat<size, size> mx;
        for (size_t i{}; i < size; ++i) mx(i, i) = value;
        return mx;
    }

      //----------------------//
     //     Псевдонимы       //
    //----------------------//

    /// <summary>
    /// Матрица размера 2х2
    /// </summary>
    using mat2x2 = mat<2, 2>;
    /// <summary>
    /// Матрица размера 3х3
    /// </summary>
    using mat3x3 = mat<3, 3>;
    /// <summary>
    /// Матрица размера 4х4
    /// </summary>
    using mat4x4 = mat<4, 4>;


    template<size_t _rows, size_t _cols>
    inline mat<_rows, _cols>& mat<_rows, _cols>::operator+=(const mat<_rows, _cols>& other)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] += other._elems[r][c];
        return *this;
    }
    template<size_t _rows, size_t _cols>
    inline mat<_rows, _cols>& mat<_rows, _cols>::operator-=(const mat<_rows, _cols>& other)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] -= other._elems[r][c];
        return *this;
    }
    template<size_t _rows, size_t _cols>
    inline mat<_rows, _cols>& mat<_rows, _cols>::operator*=(double value)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] *= value;;
        return *this;
    }
    template<size_t _rows, size_t _cols>
    inline mat<_rows, _cols>& mat<_rows, _cols>::operator/=(double value)
    {
        return (*this) *= (1 / value);
    }

    template<size_t _rows, size_t _cols>
    constexpr mat<_rows, _cols> operator+ (const mat<_rows, _cols>& left, const mat<_rows, _cols>& right) {
        mat<_rows, _cols> result{};
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                result(r, c) = left(r, c) + right(r, c);
        return result;
    }

    template<size_t _rows, size_t _cols>
    constexpr mat<_rows, _cols> operator- (const mat<_rows, _cols>& left, const mat<_rows, _cols>& right) {
        mat<_rows, _cols> result{};
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                result(r, c) = left(r, c) - right(r, c);
        return result;
    }
    
    template<size_t _rows, size_t _dims, size_t _cols>
    constexpr mat<_rows, _cols> operator* (const mat<_rows, _dims>& left, const mat<_dims, _cols>& right) {
        mat<_rows, _cols> result{};
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _dims; ++c)
                for (size_t k{}; k < _cols; ++k)
                    result(r, k) += left(r, c) * right(c, k);
        return result;
    }
    
    template<size_t _rows, size_t _cols>
    constexpr vec<_rows> operator* (const mat<_rows, _cols>& mx, const vec<_cols>& vc) {
        vec<_rows> result{};
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                result[r] += mx(r, c) * vc[c];
        return result;
    }

    template<size_t _rows, size_t _cols>
    constexpr mat<_rows, _cols> operator* (const mat<_rows, _cols>& mx, double v) {
        mat<_rows, _cols> result{};
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                result(r, c) = mx(r, c) * v;
        return result;
    }

    template<size_t _rows, size_t _cols>
    constexpr mat<_rows, _cols> operator* (double v, const mat<_rows, _cols>& mx) {
        return mx * v;
    }

    template<size_t _rows, size_t _cols>
    constexpr mat<_rows, _cols> operator/ (const mat<_rows, _cols>& mx, double v) {
        return mx * (1 / v);
    }
    
    template<size_t _rows, size_t _cols>
    std::ostream& operator<< (std::ostream& os, const mat<_rows, _cols>& mx) {
        os << "( ";
        for (size_t r{}; r < _rows; ++r) {
            os << "( ";
            for (size_t c{}; c < _cols; ++c) os << mx(r, c) << ' ';
            os << ") ";
        }
        return os << ")";
    }
    
    template<size_t _rows, size_t _cols>
    std::istream& operator>> (std::istream& is, mat<_rows, _cols>& mx) {
        for (size_t r{}; r < _rows; ++r) {
            for (size_t c{}; c < _cols; ++c) {
                is >> mx(r, c);
            }
        }
        return is;
    }
}
