#pragma once
#include <istream>
#include <ostream>

namespace math {

    /// Вектор константной размерности
    template<size_t _size>
    class vec {
        static_assert(_size > 0ull, "attempt to create empty vec");
    public:
        using value_type = double;
        using iterator = double*;
        using const_iterator = const double*;
        using reference = double&;
        using const_reference = const double&;

        double _elems[_size]{};
    public:
        [[nodiscard]] constexpr reference operator[](size_t index) {
            return _elems[index];
        }
        [[nodiscard]] constexpr const_reference operator[](size_t index) const {
            return _elems[index];
        }

        /// размер вектора
        constexpr size_t size() const noexcept { return _size; }

        /// указатель на массив значений
        double* data() noexcept { return _elems; }
        /// указатель на массив значений
        const double* data() const noexcept { return _elems; }

        iterator begin() noexcept { return _elems; }
        const_iterator begin() const noexcept { return _elems; }
        iterator end() noexcept { return _elems + _size; }
        const_iterator end() const noexcept { return _elems + _size; }

        vec& operator+= (const vec& other);
        vec& operator-= (const vec& other);
        vec& operator*= (double value);
        vec& operator/= (double value);
    };

      //------------------------------//
     //          Псевдонимы          //
    //------------------------------//

    /// 2d vector (x, y)
    using vec2 = vec<2>;
    /// 3d vector (x, y, z)
    using vec3 = vec<3>;
    /// 4d vector (x, y, z, w)
    using vec4 = vec<4>;


      //-------------------------------------------//
     // Арифметические операторации над векторами //
    //-------------------------------------------//

    template<size_t _size>
    inline vec<_size>& vec<_size>::operator+=(const vec<_size>& other)
    {
        for (size_t i{}; i < _size; ++i) _elems[i] += other[i];
        return *this;
    }

    template<size_t _size>
    inline vec<_size>& vec<_size>::operator-=(const vec<_size>& other)
    {
        for (size_t i{}; i < _size; ++i) _elems[i] -= other[i];
        return *this;
    }

    template<size_t _size>
    inline vec<_size>& vec<_size>::operator*=(double value)
    {
        for (size_t i{}; i < _size; ++i) _elems[i] *= value;
        return *this;
    }

    template<size_t _size>
    inline vec<_size>& vec<_size>::operator/=(double value)
    {
        for (size_t i{}; i < _size; ++i) _elems[i] /= value;
        return *this;
    }

    template<size_t size>
    constexpr vec<size> operator+ (const vec<size>& first, const vec<size>& second)
    {
        vec<size> result;
        for (size_t i{}; i < size; ++i) result[i] = first[i] + second[i];
        return result;
    }

    template<size_t size>
    constexpr vec<size> operator- (const vec<size>& first, const vec<size>& second) 
    {
        vec<size> result;
        for (size_t i{}; i < size; ++i) result[i] = first[i] - second[i];
        return result;
    }

    template<size_t size>
    constexpr double operator* (const vec<size>& first, const vec<size>& second) 
    {
        double result{};
        for (size_t i{}; i < size; ++i) result += first[i] * second[i];
        return result;
    }

    template<size_t size>
    constexpr vec<size> operator* (double m, const vec<size>& vc)
    {
        vec<size> result;
        for (size_t i{}; i < size; ++i) result[i] += vc[i] * m;
        return result;
    }

    template<size_t size>
    constexpr vec<size> operator* (const vec<size>& vc, double m) {
        return m * vc;
    }

    template<size_t size>
    constexpr vec<size> operator/ (const vec<size>& vc, double m) {
        return (1 / m) * vc;
    }

    /// смена знака
    template<size_t size>
    constexpr vec<size> operator- (const vec<size>& vc) noexcept {
        vec<size> result{};
        for (size_t i{}; i < size; ++i) result[i] = -vc[i];
        return result;
    }

    template<size_t size>
    std::ostream& operator<< (std::ostream& os, const vec<size>& vc) {
        os << "( ";
        for (size_t i{}; i < size; ++i) os << vc[i] << ' ';
        return os << ")";
    }
    
    template<size_t size>
    std::istream& operator>> (std::istream& is, vec<size>& vc) {
        for (double& val : vc) is >> val;
        return is;
    }

    /// вектор из единиц
    template<size_t size>
    constexpr vec<size> ones() noexcept {
        vec<size> vc;
        for (size_t i{}; i < size; ++i) vc[i] = 1.0;
        return vc;
    }


    /// получение среза вектора
    template<size_t begin, size_t end, size_t size>
    constexpr vec<end - begin + 1> slice(const vec<size> vc) noexcept {
        static_assert(end < size&& begin < end && (end - begin) < size, "indices out of bounds");
        vec<end - begin + 1> subvc;
        for (size_t i{ begin }; i <= end; ++i) subvc[i - begin] = vc[i];
        return subvc;
    }

    /// объединение двух векторов в один
    template<size_t dim1, size_t dim2> vec<dim1 + dim2>
        constexpr unite(const vec<dim1>& first, const vec<dim2>& second) noexcept {
            vec<dim1 + dim2> result;
            for (size_t i{}; i < dim1; ++i) result[i] = first[i];
            for (size_t i{}; i < dim2; ++i) result[i + dim1] = second[i];
            return result;
        }

}
