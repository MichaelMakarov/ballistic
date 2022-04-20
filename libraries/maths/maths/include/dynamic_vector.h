#pragma once
#include <ostream>
#include <istream>

namespace math {
    /// <summary>
    /// Vector of dynamic size
    /// </summary>
    class vector {
    public:
        using value_type = double;
        using iterator = double*;
        using const_iterator = const double*;
        using reference = double&;
        using const_reference = const double&;
    private:
        double* _elems;
        size_t _size;
    private:
        void copy(const vector& other);
        void move(vector& other) noexcept;
    public:
        /// <summary>
        /// Creates an empty vector.
        /// </summary>
        vector() : _elems{ nullptr }, _size{} {}
        /// <summary>
        /// Creates vector of specified size which elements are zeros.
        /// </summary>
        /// <param name="size">is size of vector</param>
        explicit vector(size_t size);
        /// <summary>
        /// Creates vector of specified size and fills specified value.
        /// </summary>
        /// <param name="size">is size of vector</param>
        /// <param name="value">is a value of elements</param>
        vector(size_t size, double value);
        /// <summary>
        /// Creating from initializer list.
        /// </summary>
        /// <param name="list"></param>
        vector(std::initializer_list<double> list);
        vector(const vector& other);
        vector(vector&& other) noexcept;
        ~vector();

        vector& operator= (const vector& other);
        vector& operator= (vector&& other) noexcept;

        double& operator[] (size_t index) { return _elems[index]; }
        const double& operator[] (size_t index) const { return _elems[index]; }

        /// <summary>
        /// Returns size of the vector.
        /// </summary>
        /// <returns></returns>
        size_t size() const noexcept { return _size; }

        double* data() { return _elems; }
        const double* data() const { return _elems; }

        double* begin() noexcept { return data(); }
        const double* begin() const noexcept { return data(); }
        double* end() noexcept { return data() + _size; }
        const double* end() const noexcept { return data() + _size; }

        /// <summary>
        /// Returns the first element.
        /// </summary>
        /// <returns>reference to element</returns>
        double& first();
        /// <summary>
        /// Returns the first element.
        /// </summary>
        /// <returns>constant reference to element</returns>
        const double& first() const;
        /// <summary>
        /// Returns the last element.
        /// </summary>
        /// <returns>reference to element</returns>
        double& last();
        /// <summary>
        /// Returns the last element.
        /// </summary>
        /// <returns>constant reference to element</returns>
        const double& last() const;

        vector& operator+= (const vector& other);
        vector& operator-= (const vector& other);
        vector& operator*= (double value);
        vector& operator/= (double value);

        friend vector operator+ (const vector& left, const vector& right);
        friend vector operator- (const vector& left, const vector& right);
        friend double operator* (const vector& left, const vector& right);
        friend vector operator* (double value, const vector& vec);
        friend vector operator* (const vector& vec, double value);
        friend vector operator/ (const vector& vec, double value);

        friend std::ostream& operator<< (std::ostream& os, const vector& vec);
        friend std::istream& operator>> (std::istream& is, vector& vec);
    };
}
