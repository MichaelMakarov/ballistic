#include <maths.hpp>
#include <vector>
#include <cstring>
#ifdef __AVX__
#include <immintrin.h>
#endif

void (*add_ptr)(const double *, const double *, double *, size_t);
void (*sub_ptr)(const double *, const double *, double *, size_t);
void (*mul_ptr)(const double *, double, double *, size_t);
void (*mulmm_ptr)(const double *, const double *, double *, size_t, size_t, size_t);
void (*mulvv_ptr)(const double *, const double *, double &, size_t);
void (*mulmv_ptr)(const double *, const double *, double *, size_t, size_t);

constexpr double zero{1e-20};
constexpr size_t allign{256 / 8 / sizeof(double)};

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
    for (size_t i{}; i < size; ++i)
    {
        auto mul_md = _mm256_mul_pd(_mm256_load_pd(left), _mm256_load_pd(right));
        _mm256_add_pd(sum_md, mul_md);
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

bool initialize_functions()
{
#ifdef __AVX__
    bool avxsupport();
    const bool avx = avxsupport();
    if (avx)
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
    return true;
}

const bool ___ = initialize_functions();

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

#define throw_on_mult throw std::invalid_argument("У перемножаемых матриц кол-во столбцов левой матрицы не равно кол-ву строк правой.");

matrix &matrix::operator+=(const matrix &other)
{
    if (!check_dimension(*this, other))
    {
        throw std::invalid_argument("Складываемая матрица имеет другую размерность.");
    }
    add_ptr(_elems, other._elems, _elems, _rows * _cols);
    return *this;
}

matrix &matrix::operator-=(const matrix &other)
{
    if (!check_dimension(*this, other))
    {
        throw std::invalid_argument("Вычитаемая матрица имеет другую размерность.");
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
        throw std::invalid_argument("Кол-во столбцов матрицы не равно размерности вектора.");
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

#define throw_on_notsq throw std::invalid_argument("Матрица не является квадратной.");

void inverse(matrix &mx, double *determ)
{
    size_t rows = mx.rows(), cols = mx.columns();
    if (rows != cols)
    {
        throw_on_notsq
    }
    auto &inv = mx;
    double pivot, det{1};
    std::vector<size_t> descent(rows);
    size_t k{};
    for (size_t r{}; r < rows; ++r)
        descent[r] = r;
    for (size_t i{}; i < rows; ++i)
    {
        k = descent[i];
        pivot = inv[k][k];
        if (std::fabs(pivot) < zero)
        {
            if (i < rows - 1)
            {
                throw std::runtime_error("Матрица является вырожденной и не может быть обращена.");
            }
            std::swap(descent[i], descent[i + 1]);
            k = descent[i];
            pivot = inv[k][k];
            det = -det;
        }
        det *= pivot;
        for (size_t r{}; r < rows; ++r)
            inv[r][k] /= -pivot;
        for (size_t r{}; r < rows; ++r)
        {
            if (r != k)
            {
                for (size_t c{}; c < cols; ++c)
                {
                    if (c != k)
                        inv[r][c] += inv[k][c] * inv[r][k];
                }
            }
        }
        for (size_t c{}; c < cols; ++c)
            inv[k][c] /= pivot;
        inv[k][k] = 1 / pivot;
    }
    if (determ)
        *determ = det;
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
        throw std::invalid_argument("Добавляемый вектор имеет другую размерность.");
    }
    add_ptr(_elems, other._elems, _elems, _size);
    return *this;
}

vector &vector::operator-=(const vector &other)
{
    if (_size != other._size)
    {
        throw std::invalid_argument("Вычитаемый вектор имеет другую размерность.");
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
        throw std::invalid_argument("Перемножаемые векторы имеют разные размерности.");
    }
    double result{};
    mulvv_ptr(left._elems, right._elems, result, left._size);
    return result;
    ;
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

#define throw_if_empty throw std::out_of_range("Пустой вектор не имеет элементов.");

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

vector lstsq(const matrix &mx, const vector &vc)
{
    auto sysm = mx * transpose(mx);
    size_t rows = sysm.rows();
    vector diag(rows);
    for (size_t i{}; i < rows; ++i)
        diag[i] = 1 / std::sqrt(sysm[i][i]);
    mxd(sysm, diag);
    dxm(diag, sysm);
    inverse(sysm);
    mxd(sysm, diag);
    dxm(diag, sysm);
    return sysm * (mx * vc);
}
