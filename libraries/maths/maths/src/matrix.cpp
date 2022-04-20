#include <dynamic_matrix.h>
#include <arithmetics.h>
#include <assertion.h>
#include <arithmetics.h>
#include <vector>
#include <cstring>
#include <iterator>

constexpr double ZERO{ 1e-20 };

namespace math {

    void matrix::copy(const matrix& other) {
        _rows = other._rows;
        _cols = other._cols;
        _elems = new double[_rows * _cols];
        _ptrs = new double* [_rows];
        for (size_t i{}; i < _rows; ++i) _ptrs[i] = _elems + i * _cols;
        std::memcpy(_elems, other._elems, _rows * _cols * sizeof(double));
    }

    void matrix::move(matrix& other) noexcept {
        std::swap(_rows, other._rows);
        std::swap(_cols, other._cols);
        std::swap(_elems, other._elems);
        std::swap(_ptrs, other._ptrs);
    }
    
    bool check_dimension(const matrix& left, const matrix& right) {
        return left._rows == right._rows && left._cols == right._cols;
    }

    matrix::matrix(const size_t rows, const size_t columns) {
        _rows = rows;
        _cols = columns;
        size_t cols = _cols;
        _elems = new double[_rows * cols]{};
        _ptrs = new double* [_rows];
        for (size_t i{}; i < _rows; ++i) _ptrs[i] = _elems + i * cols;
    }

    matrix::matrix(std::initializer_list<std::initializer_list<double>> lists) {
        _rows = lists.size();
        _cols = 0;
        for (const auto& list : lists) {
            if ((size_t)list.size() > _cols) _cols = list.size();
        }
        _elems = new double[_rows * _cols]{};
        _ptrs = new double* [_rows];
        size_t i{};
        for (const auto& list : lists) {
            _ptrs[i] = _elems + i * _cols;
            std::copy(list.begin(), list.end(), _ptrs[i++]);
        }
    }

    matrix::matrix(const matrix& other) {
        copy(other);
    }

    matrix::matrix(matrix&& other) noexcept : matrix() {
        move(other);
    }

    matrix::~matrix() {
        if (_elems) {
            delete[] _elems;
            delete[] _ptrs;
            _elems = nullptr;
            _ptrs = nullptr;
        }
    }

    matrix& matrix::operator= (const matrix& other) {
        copy(other);
        return *this;
    }

    matrix& matrix::operator= (matrix&& other) noexcept {
        move(other);
        return *this;
    }

    vector matrix::get_row(size_t index) const  {
        ASSERT(index < _rows, "row index out of range");
        vector row(_cols);
        std::memcpy(row.data(), _ptrs[index], sizeof(double) * _cols);
        return row;
    }

    vector matrix::get_column(size_t index) const  {
        ASSERT(index < _cols, "column index out of range");
        auto column = vector(_rows);
        for (size_t r{}; r < _rows; ++r) column[r] = _ptrs[r][index];
        return column;
    }

    void matrix::set_row(size_t index, const vector& row)  {
        ASSERT(index < _rows, "row index out of range");
        ASSERT(_cols == row.size(), "invalid vector size");
        std::memcpy(_ptrs[index], row.data(), sizeof(double) * _cols);
    }

    void matrix::set_column(size_t index, const vector& column)  {
        ASSERT(index < _cols, "column index out of range");
        ASSERT(_rows == column.size(), "invalid vector size");
        for (size_t r{}; r < _rows; ++r) _ptrs[r][index] = column[r];
    }

    matrix& matrix::operator+= (const matrix& other)  {
        ASSERT(check_dimension(*this, other), "expected matrix of the same dimensions");

        size_t size = _rows * _cols;
        for (size_t i{}; i < size; ++i) _elems[i] += other._elems[i];

        return *this;
    }

    matrix& matrix::operator-= (const matrix& other)  {
        ASSERT(check_dimension(*this, other), "expected matrix of the same dimensions");

        size_t size = _rows * _cols;
        for (size_t i{}; i < size; ++i) _elems[i] -= other._elems[i];

        return *this;
    }

    matrix& matrix::operator*= (double value) {
        size_t size = _rows * _cols;
        for (size_t i{}; i < size; ++i) _elems[i] *= value;

        return *this;
    }

    matrix& matrix::operator/= (double value) {
        size_t size = _rows * _cols;
        for (size_t i{}; i < size; ++i) _elems[i] /= value;

        return *this;
    }

    matrix operator* (const matrix& left, const matrix& right)  {
        ASSERT(left._cols == right._rows, "incompatible matrices for multiplication");

        matrix result(left._rows, right._cols);
        for (size_t r{}; r < left._rows; ++r)
            for (size_t k{}; k < left._cols; ++k)
                for (size_t c{}; c < right._cols; ++c)
                    result._ptrs[r][c] += left._ptrs[r][k] * right._ptrs[k][c];

        return result;
    }

    vector operator* (const matrix& mx, const vector& vec)  {
        ASSERT(mx._cols == vec.size(), "incompatible matrix and vector for multiplication");

        vector result(mx._rows);
        for (size_t r{}; r < mx._rows; ++r) {
            double& result_value = result[r];
            for (size_t c{}; c < mx._cols; ++c)
                result_value += mx._ptrs[r][c] * vec[c];
        }
        return result;
    }

    matrix operator+ (const matrix& left, const matrix& right) {
        return matrix(left) += right;
    }

    matrix operator- (const matrix& left, const matrix& right)  {
        return matrix(left) -= right;
    }


    matrix operator/ (const matrix& mx, double value) {
        return matrix(mx) /= value;
    }

    matrix operator* (const matrix& mx, double value) {
        return matrix(mx) *= value;
    }

    matrix operator* (double value, const matrix& mx) {
        return matrix(mx) *= value;
    }

    std::ostream& operator<< (std::ostream& os, const matrix& mx) {
        os << "( ";
        for (size_t r{}; r < mx._rows; ++r) {
            os << "( ";
            std::copy(mx._ptrs[r], mx._ptrs[r] + mx._cols, std::ostream_iterator<double>(os, " "));
            os << ") ";
        }
        return os << ")";
    }

    std::istream& operator>> (std::istream& is, matrix& mx) {
        for (size_t i{}; i < mx._rows * mx._cols; ++i) is >> mx._elems[i];
        return is;
    }


    matrix diag(size_t dim, double value) {
        matrix mx(dim, dim);
        for (size_t i{}; i < dim; ++i) mx(i, i) = value;
        return mx;
    }

    matrix transpose(const matrix& mx) {
        size_t rows = mx.rows(), cols = mx.columns();
        matrix tr(cols, rows);
        for (size_t r{}; r < rows; ++r) {
            for (size_t c{}; c < cols; ++c) {
                tr(c, r) = mx(r, c);
            }
        }
        return tr;
    }

    void inverse(matrix& mx, double* determ) {
        size_t rows = mx.rows(), cols = mx.columns();
        ASSERT(rows == cols, "non square matrix can not be inverted");

        auto& inv = mx;
        double pivot, det{ 1 };
        std::vector<size_t> descent(rows);
        size_t k{};

        for (size_t r{}; r < rows; ++r) descent[r] = r;
        for (size_t i{}; i < rows; ++i) {
            k = descent[i];
            pivot = inv(k, k);
            if (math::abs(pivot) < ZERO) {
                ASSERT(i < rows - 1, "degenerate matrix can not be inverted");
                std::swap(descent[i], descent[i + 1]);
                k = descent[i];
                pivot = inv(k, k);
                det = -det;
            }
            det *= pivot;
            for (size_t r{}; r < rows; ++r) inv(r, k) /= -pivot;
            for (size_t r{}; r < rows; ++r) {
                if (r != k) {
                    for (size_t c{}; c < cols; ++c) {
                        if (c != k)
                            inv(r, c) += inv(k, c) * inv(r, k);
                    }
                }
            }
            for (size_t c{}; c < cols; ++c) inv(k, c) /= pivot;
            inv(k, k) = 1 / pivot;
        }

        if (determ) *determ = det;
    }

    void lu(const matrix& mx, matrix& lower, matrix& upper)  {
        size_t rows = mx.rows(), cols = mx.columns();
        ASSERT(rows == cols, "matrix is not square");

        upper = lower = matrix(rows, cols);
        double sum;

        for (size_t r{}; r < rows; ++r) {
            for (size_t c{}; c < cols; ++c) {
                sum = 0;
                for (size_t i{}; i < cols; ++i)
                    sum += lower(r, i) * upper(i, c);
                if (r <= c) {
                    upper(r, c) = mx(r, c) - sum;
                } else {
                    lower(r, c) = (mx(r, c) - sum) / upper(r, c);
                }
            }
            lower(r, r) = 1.0;
        }
    }

    void normalize(vector& v);

    void qr(const matrix& mx, matrix& orth, matrix& upper)  {
        size_t rows = mx.rows(), cols = mx.columns();
        ASSERT(rows == cols, "matrix is not square");

        orth = matrix(rows, cols);
        std::vector<vector> vecs(cols);

        for (size_t n{}; n < cols; ++n) {
            vecs[n] = mx.get_column(n);
            // Gramme-Shmidte orthogonalization
            for (size_t k{}; k < n; ++k) {
                vecs[n] -= vecs[k] * (vecs[n] * vecs[k]);
            }
            normalize(vecs[n]);
            orth.set_column(n, vecs[n]);
        }
        upper = transpose(orth) * mx;
    }

    void mxd(matrix& mx, const vector& diag)  {
        size_t rows = mx.rows(), cols = mx.columns();
        ASSERT(cols == diag.size(), "incompatible ordinary and diagonal matrices");

        for (size_t r{}; r < rows; ++r)
            for (size_t c{}; c < cols; ++c)
                mx(r, c) *= diag[c];
    }

    void dxm(const vector& diag, matrix& mx)  {
        size_t rows = mx.rows(), cols = mx.columns();
        ASSERT(rows == diag.size(), "incompatible ordinary and diagonal matrices");

        for (size_t r{}; r < rows; ++r)
            for (size_t c{}; c < cols; ++c)
                mx(r, c) *= diag[r];
    }

    void svd(const matrix& mx, matrix& u_mx, vector& s_mx, matrix& v_mx) {
        size_t rows = mx.rows(), cols = mx.columns();
        v_mx = matrix(cols, cols);
        s_mx = vector(cols);
        auto s_inv = vector(cols);
        auto a_mx = transpose(mx) * mx;
        vector vec(cols);
        double curr, prev;

        for (size_t i = 0; i < cols; ++i) {
            curr = 2.0;
            prev = 1.0;
            for (size_t n = 0; n < cols; ++n) vec[n] = 1.0;
            while (!is_equal(prev, curr)) {
                prev = curr;
                vec = a_mx * vec;
                normalize(vec);
                curr = vec[0];
            }
            curr = vec * (a_mx * vec);
            // filling the matrix of the eigen vectors
            v_mx.set_row(i, vec);
            // filling the diagonal matrix
            s_mx[i] = std::sqrt(curr);
            for (size_t r = 0; r < cols; ++r)
                for (size_t n = 0; n < cols; ++n)
                    a_mx(r, n) -= curr * vec[r] * vec[n];
        }
        for (size_t i{}; i < cols; ++i) s_inv[i] = 1 / s_mx[i];
        u_mx = mx * transpose(v_mx);
        mxd(u_mx, s_inv);
    }
	
}
