#include <linalg.h>
#include <assertion.h>
#include <arithmetics.h>
#include <vector>
#include <cstring>
#include <iterator>

constexpr double ZERO{ 1e-20 };

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

matrix::matrix() noexcept
{
	_rows = _cols = 0;
	_elems = nullptr;
	_ptrs = nullptr;
}

matrix::matrix(const size_t rows, const size_t columns)
{
	_rows = rows;
	_cols = columns;
	_elems = new double[_rows * _cols]{};
	_ptrs = new double* [_rows];
	for (size_t i{}; i < _rows; ++i) _ptrs[i] = _elems + i * _cols;	
}

matrix::matrix(const matrix& other) {
	this->copy(other);
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

const char* mx_same_size{ "ожидались матрицы одинаковых размерностей" };

matrix& matrix::operator+= (const matrix& other)  {
	ASSERT(check_dimension(*this, other), mx_same_size);

	size_t size = _rows * _cols;
	for (size_t i{}; i < size; ++i) _elems[i] += other._elems[i];

	return *this;
}

matrix& matrix::operator-= (const matrix& other)  {
	ASSERT(check_dimension(*this, other), mx_same_size);

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
	ASSERT(left._cols == right._rows, "матрицы несовместимы для перемножения");

	matrix result(left._rows, right._cols);
	for (size_t r{}; r < left._rows; ++r)
		for (size_t k{}; k < left._cols; ++k)
			for (size_t c{}; c < right._cols; ++c)
				result._ptrs[r][c] += left._ptrs[r][k] * right._ptrs[k][c];

	return result;
}

vector operator* (const matrix& mx, const vector& vc)  {
	ASSERT(mx._cols == vc.size(), "матрица и вектор несовместимы для перемножения");

	vector res(mx._rows);
	for (size_t r{}; r < mx._rows; ++r) {
		double& result_value = res[r];
		for (size_t c{}; c < mx._cols; ++c)
			result_value += mx._ptrs[r][c] * vc[c];
	}
	return res;
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

// std::ostream& operator<< (std::ostream& os, const matrix& mx) {
// 	os << "( ";
// 	for (size_t r{}; r < mx._rows; ++r) {
// 		os << "( ";
// 		std::copy(mx._ptrs[r], mx._ptrs[r] + mx._cols, std::ostream_iterator<double>(os, " "));
// 		os << ") ";
// 	}
// 	return os << ")";
// }

// std::istream& operator>> (std::istream& is, matrix& mx) {
// 	for (size_t i{}; i < mx._rows * mx._cols; ++i) is >> mx._elems[i];
// 	return is;
// }


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

const char* nonsquare{ "матрица не является квадратной" };

void inverse(matrix& mx, double* determ) {
	size_t rows = mx.rows(), cols = mx.columns();
	ASSERT(rows == cols, nonsquare);

	auto& inv = mx;
	double pivot, det{ 1 };
	std::vector<size_t> descent(rows);
	size_t k{};

	for (size_t r{}; r < rows; ++r) descent[r] = r;
	for (size_t i{}; i < rows; ++i) {
		k = descent[i];
		pivot = inv(k, k);
		if (std::fabs(pivot) < ZERO) {
			ASSERT(i < rows - 1, "матрица является вырожденной");
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
	ASSERT(rows == cols, nonsquare);

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

const char* diagerror{ "матрица несовместима с диагональной для перемножения" };

void mxd(matrix& mx, const vector& diag)  {
	size_t rows = mx.rows(), cols = mx.columns();
	ASSERT(cols == diag.size(), diagerror);

	for (size_t r{}; r < rows; ++r)
		for (size_t c{}; c < cols; ++c)
			mx(r, c) *= diag[c];
}

void dxm(const vector& diag, matrix& mx)  {
	size_t rows = mx.rows(), cols = mx.columns();
	ASSERT(rows == diag.size(), diagerror);

	for (size_t r{}; r < rows; ++r)
		for (size_t c{}; c < cols; ++c)
			mx(r, c) *= diag[r];
}
