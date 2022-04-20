#pragma once
#include "dynamic_vector.h"

namespace math {

    /// <summary>
    /// Matrix of dynamic size
    /// </summary>
    class matrix {
    private:
        double* _elems;
        double** _ptrs;
        size_t _rows, _cols;
    private:
        void copy(const matrix& other);
        void move(matrix& other) noexcept;
        friend inline bool check_dimension(const matrix& left, const matrix& right);
    public:
        /// <summary>
        /// Creates an empty matrix.
        /// </summary>
        matrix() : _elems{ nullptr }, _ptrs{ nullptr }, _rows{}, _cols{} {}
        /// <summary>
        /// Creates matrix of specified dimensions which elements are zeros.
        /// </summary>
        /// <param name="rows">is a number of rows</param>
        /// <param name="columns">is a number of columns</param>
        matrix(size_t rows, size_t columns);
        /// <summary>
        /// Creates from initializer list.
        /// </summary>
        /// <param name="list"></param>
        matrix(std::initializer_list<std::initializer_list<double>> list);
        matrix(const matrix& other);
        matrix(matrix&& other) noexcept;
        ~matrix();

        matrix& operator= (const matrix& other);
        matrix& operator= (matrix&& other) noexcept;

        /// <summary>
        /// Returns a number of rows.
        /// </summary>
        /// <returns></returns>
        size_t rows() const noexcept { return _rows; }
        /// <summary>
        /// Returns a number of columns.
        /// </summary>
        /// <returns></returns>
        size_t columns() const noexcept { return _cols; }

        double** data() noexcept { return _ptrs; }
        const double* const* data() const noexcept { return _ptrs; }

        double& operator() (size_t row, size_t col) { return _ptrs[row][col]; }
        const double& operator() (size_t row, size_t col) const { return _ptrs[row][col]; }

        /// <summary>
        /// Returns a row of matrix by index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        vector get_row(size_t index) const;
        /// <summary>
        /// Returns a column of matrix by index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        vector get_column(size_t index) const;
        /// <summary>
        /// Fills the elements of row of matrix.
        /// </summary>
        /// <param name="index">is index of row</param>
        /// <param name="row">is a vector of elements</param>
        void set_row(size_t index, const vector& row);
        /// <summary>
        /// Fills the elements of column of matrix.
        /// </summary>
        /// <param name="index">is index of column</param>
        /// <param name="column">is a vector of elements</param>
        void set_column(size_t index, const vector& column);

        matrix& operator+= (const matrix& other);
        matrix& operator-= (const matrix& other);
        matrix& operator*= (double v);
        matrix& operator/= (double v);

        friend matrix operator+ (const matrix& left, const matrix& right);
        friend matrix operator- (const matrix& left, const matrix& right);
        friend matrix operator* (const matrix& left, const matrix& right);
        friend vector operator* (const matrix& mx, const vector& vec);
        friend matrix operator/ (const matrix& mx, double value);
        friend matrix operator* (const matrix& mx, double value);
        friend matrix operator* (double value, const matrix& mx);

        friend std::ostream& operator<< (std::ostream& os, const matrix& mx);
        friend std::istream& operator>> (std::istream& is, matrix& mx);

        /// <summary>
        /// Returns diagonal square matrix with specified elements of the diagonal.
        /// </summary>
        /// <param name="size">is size of matrix</param>
        /// <param name="value">is a value of diagonal elements</param>
        /// <returns></returns>
        friend matrix diag(const size_t size, double value = double{ 1 });
    };
}
