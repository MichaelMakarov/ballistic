#pragma once
#include <cstddef>

/**
 * @brief Итерируемый двумерный массив
 *
 * @tparam _rows кол-во строк
 * @tparam _cols кол-во столбцов
 */
template<size_t _rows, size_t _cols>
struct basic_array {
	static_assert(_rows * _cols > 0, "массив не может иметь нулевое измерение");

	using value_type = double;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = value_type*;
	using const_iterator = const value_type*;

	double _elems[_rows][_cols]{};

	double* begin() { return _elems[0]; }
	const double* begin() const { return _elems[0]; }
	double* end() { return _elems[_rows]; }
	const double* end() const { return _elems[_rows]; }

};

/**
 * @brief Индексируемый двумерный массив
 *
 * @tparam _rows кол-во строк
 * @tparam _cols кол-во столбцов
 */
template<size_t _rows, size_t _cols>
struct indexed_array : basic_array<_rows, _cols> {

	[[nodiscard]] constexpr double& operator()(size_t r, size_t c) {
		return _elems[r][c];
	}
	[[nodiscard]] constexpr const double& operator()(size_t r, size_t c) const {
		return _elems[r][c];
	}

	/**
	 * @brief Кол-во строк
	 */
	constexpr size_t rows() const { return _rows; }
	/**
	 * @brief Кол-во столбцов
	 */
	constexpr size_t columns() const { return _cols; }
	/**
	 * @brief Указатель на начало двумерного массива
	 */
	double** data() { return _elems; }
	/**
	 * @brief Указатель на начало двумерного массива
	 */
	const double** data() const { return _elems; }

};

/**
 * @brief Индексируемый одномерный массив
 *
 * @tparam _size размер массива
 */
template<size_t _size>
struct indexed_array<_size, 1> : basic_array<_size, 1> {

	[[nodiscard]] constexpr double& operator[](size_t i) {
		return _elems[i][0];
	}
	[[nodiscard]] constexpr const double& operator[](size_t i) const {
		return _elems[i][0];
	}

	/**
	 * @brief Кол-во элементов
	 */
	constexpr size_t size() const { return _size; }
	/**
	 * @brief Указатель на начало массива
	 */
	double* data() { return _elems[0]; }
	/**
	 * @brief Указатель на начало массива
	 */
	const double* data() const { return _elems[0]; }

};

/**
 * @brief Двумерный массив (с арифметическими операциями)
 *
 * @tparam _rows кол-во строк
 * @tparam _cols кол-во столбцов
 */
template<size_t _rows, size_t _cols>
struct numeric_array : indexed_array<_rows, _cols> {

	numeric_array& operator+= (const numeric_array& other);
	numeric_array& operator-= (const numeric_array& other);
	numeric_array& operator*= (double other);
	numeric_array& operator/= (double other);

};

/**
 * @brief Вектор
 *
 * @tparam size размер вектора
 */
template<size_t size>
using vec = numeric_array<size, 1>;
/**
 * @brief Матрица
 *
 * @tparam rows кол-во строк
 * @tparam cols кол-во столбцов
 */
template<size_t rows, size_t cols>
using mat = numeric_array<rows, cols>;

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
using vec6 = vec<6>;
using mat2x2 = mat<2, 2>;
using mat3x3 = mat<3, 3>;
using mat4x4 = mat<4, 4>;

/**
 * @brief Извлечение подмножества вектора
 *
 * @tparam _beg нач. индекс
 * @tparam _end кон. индекс
 * @tparam _size размер исходного вектора
 * @param v исходный вектор
 * @return вектор vec<_end - _beg + 1>
 */
template<size_t _beg, size_t _end, size_t _size>
constexpr vec<_end - _beg + 1> slice(const vec<_size>& v) {
	static_assert(_end < _size, "кон. индекс должен быть меньше размера вектора");
	static_assert(_beg < _end, "начю индекс должен быть меньше кон. индекса");
	vec<_end - _beg + 1> r;
	for (size_t i{ _beg }, j{}; i <= _end; ++i, ++j) {
		r[j] = v[i];
	}
	return r;
}

template<size_t _rows, size_t _cols>
inline numeric_array<_rows, _cols>& numeric_array<_rows, _cols>::operator+=(const numeric_array<_rows, _cols>& other)
{
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			_elems[r][c] += other._elems[r][c];
	return *this;
}

template<size_t _rows, size_t _cols>
inline numeric_array<_rows, _cols>& numeric_array<_rows, _cols>::operator-=(const numeric_array<_rows, _cols>& other)
{
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			_elems[r][c] -= other._elems[r][c];
	return *this;
}

template<size_t _rows, size_t _cols>
inline numeric_array<_rows, _cols>& numeric_array<_rows, _cols>::operator*=(double value)
{
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			_elems[r][c] *= value;;
	return *this;
}

template<size_t _rows, size_t _cols>
inline numeric_array<_rows, _cols>& numeric_array<_rows, _cols>::operator/=(double value)
{
	return (*this) *= (1 / value);
}

template<size_t _size>
constexpr numeric_array<_size, 1> operator-(const numeric_array<_size, 1>& arr)
{
	numeric_array<_size, 1> res;
	for (size_t i{}; i < _size; ++i) res[i] = -arr[i];
	return res;
}

template<size_t _rows, size_t _cols>
constexpr numeric_array<_rows, _cols> operator+(const numeric_array<_rows, _cols>& left, const numeric_array<_rows, _cols>& right)
{
	numeric_array<_rows, _cols> result{};
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			result._elems[r][c] = left._elems[r][c] + right._elems[r][c];
	return result;
}

template<size_t _rows, size_t _cols>
constexpr numeric_array<_rows, _cols> operator-(const numeric_array<_rows, _cols>& left, const numeric_array<_rows, _cols>& right)
{
	numeric_array<_rows, _cols> result{};
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			result._elems[r][c] = left._elems[r][c] - right._elems[r][c];
	return result;
}

template<size_t _rows, size_t _dims, size_t _cols>
constexpr numeric_array<_rows, _cols> operator*(const numeric_array<_rows, _dims>& left, const numeric_array<_dims, _cols>& right)
{
	numeric_array<_rows, _cols> result{};
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _dims; ++c)
			for (size_t k{}; k < _cols; ++k)
				result._elems[r][k] += left._elems[r][c] * right._elems[c][k];
	return result;
}

template<size_t _size>
constexpr double operator*(const numeric_array<_size, 1>& left, const numeric_array<_size, 1>& right)
{
	double val{};
	for (size_t i{}; i < _size; ++i) {
		val += left[i] * right[i];
	}
	return val;
}

template<size_t _rows, size_t _cols>
constexpr numeric_array<_rows, _cols> operator*(const numeric_array<_rows, _cols>& mx, double v)
{
	numeric_array<_rows, _cols> result{};
	for (size_t r{}; r < _rows; ++r)
		for (size_t c{}; c < _cols; ++c)
			result._elems[r][c] = mx._elems[r][c] * v;
	return result;
}

template<size_t _rows, size_t _cols>
constexpr numeric_array<_rows, _cols> operator*(double v, const numeric_array<_rows, _cols>& mx)
{
	return mx * v;
}

template<size_t _rows, size_t _cols>
constexpr numeric_array<_rows, _cols> operator/(const numeric_array<_rows, _cols>& mx, double v)
{
	return mx * (1 / v);
}

/**
 * @brief Длина вектора
 *
 * @tparam _size размерность вектора
 * @param v исходный вектор
 * @return double длина
 */
template<size_t _size>
double length(const vec<_size>& v) { return std::sqrt(v * v); }
/**
 * @brief Нормализация вектора
 *
 * @tparam _size размерность вектора
 * @param v исходный вектор
 */
template<size_t _size>
void normalize(vec<_size>& v) { v /= length(v); }
/**
 * @brief Скалярное произведение двух векторов
 *
 * @param left левый
 * @param right правый
 * @return результирующий вектор
 */
constexpr vec3 cross(const vec3& left, const vec3& right) {
	return {
		left[1] * right[2] - left[2] * right[1],
		left[2] * right[0] - left[0] * right[2],
		left[0] * right[1] - left[1] * right[0]
	};
}
/**
 * @brief Построение матрицы перехода в систему координат по заданному вектору
 *
 * @param v вектор, задающий направление для первого базисного вектора
 * @return mat3x3 матрица перехода
 */
mat3x3 make_transform(vec3 v);

/**
 * @brief Вектор динамической размерности
 *
 */
class vector {
	double* _elems;
	size_t _size;
private:
	void copy(const vector& other);
	void move(vector& other) noexcept;
public:
	using value_type = double;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = value_type*;
	using const_iterator = const value_type*;
public:
	vector() noexcept;
	vector(size_t size);
	vector(size_t size, double value);
	vector(const vector& other);
	vector(vector&& other) noexcept;
	~vector();

	vector& operator=(const vector& other);
	vector& operator=(vector&& other) noexcept;

	size_t size() const noexcept { return _size; }

	double& operator[](size_t i) {
		return _elems[i];
	}
	const double& operator[](size_t i) const {
		return _elems[i];
	}

	double* data() { return _elems; }
	const double* data() const { return _elems; }

	double& first();
	const double& first() const;
	double& last();
	const double& last() const;

	double* begin();
	const double* begin() const;
	double* end();
	const double* end() const;

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

};

/**
 * @brief Матрица динамической размерности
 *
 */
class matrix {
private:
	double* _elems;
	double** _ptrs;
	size_t _rows, _cols;
private:
	void copy(const matrix& other);
	void move(matrix& other) noexcept;
	friend bool check_dimension(const matrix& left, const matrix& right);
public:
	matrix() noexcept;
	matrix(size_t rows, size_t columns);
	matrix(const matrix& other);
	matrix(matrix&& other) noexcept;
	~matrix();

	matrix& operator= (const matrix& other);
	matrix& operator= (matrix&& other) noexcept;

	size_t rows() const noexcept { return _rows; }
	size_t columns() const noexcept { return _cols; }

	double** data() noexcept { return _ptrs; }
	const double* const* data() const noexcept { return _ptrs; }

	double& operator() (size_t row, size_t col) {
		return _ptrs[row][col];
	}
	const double& operator() (size_t row, size_t col) const {
		return _ptrs[row][col];
	}

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

};

/**
 * @brief Вычисление длины вектора
 *
 * @param v исходный вектор
 * @return длина вектора
 */
double length(const vector& v);
/**
 * @brief Нормализация вектора
 *
 * @param v исходный вектор
 */
void normalize(vector& v);
/**
 * @brief Вычисление транспонированной матрицы
 *
 * @param m исходная матрица
 * @return транспонированная матрица
 */
matrix transpose(const matrix& m);
/**
 * @brief Вычисление обратной матрицы
 *
 * @param mx исходная матрица
 * @param det определитель матрицы
 */
void inverse(matrix& mx, double* det = nullptr);
/**
 * @brief Умножение матрицы на диагональную справа
 *
 * @param mx исходная матрица
 * @param diag диагональная матрица
 */
void mxd(matrix& mx, const vector& diag);
/**
 * @brief Умножение матрицы на диагональную слева
 *
 * @param diag диагональная матрица
 * @param mx исходная матрица
 */
void dxm(const vector& diag, matrix& mx);
/**
 * @brief Решение задачи МНК
 *
 * @param mx матрица системы nxm
 * @param vc вектор правой части mx1
 * @return решение - вектор nx1
 */
vector lstsq(const matrix& mx, const vector& vc);


/**
 * @brief Степенной полином
 *
 * @tparam _degree степень
 */
template<size_t _degree>
struct polynomial {

	double _elems[_degree + 1]{};

	constexpr double operator()(double x) const {
		double mult{ x }, total{ _elems[0] };
		for (size_t i{ 1 }; i <= _degree; ++i) {
			total += mult * _elems[i];
			mult *= x;
		}
		return total;
	}

	double* data() { return _elems; }
	const double* data() const { return _elems; }

};

/**
 * @brief Аппроксимация полиномом массива точек
 *
 * @tparam degree степенб полинома
 * @tparam forw_it итератор последовательного доступа
 * @param x итератор на начало массива координат х
 * @param y итератор на начало массива координат у
 * @param count кол-во точек
 * @return аппроксимирующий полином
 */
template<
	size_t degree, typename forw_it,
	typename O = typename std::enable_if<std::is_same_v<typename std::iterator_traits<forw_it>::value_type, double>>::type>
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
/**
 * @brief Аппроксимация полиномом массива точек
 *
 * @tparam degree степенб полинома
 * @tparam forw_it итератор последовательного доступа
 * @param begin итератор на начало массива точек
 * @param end итератор на конец массива точек
 * @return аппроксимирующий полином
 */
template<
	size_t degree, typename forw_it,
	typename O = typename std::enable_if<std::is_same_v<typename std::iterator_traits<forw_it>::value_type, vec2>>::type>
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

/**
 * @brief Параметры поворота
 * 
 */
struct rotation_desc {
	/**
	 * @brief Ось вращения
	 * 
	 */
	vec3 axis;
	/**
	 * @brief Угол поворота
	 * 
	 */
	double angle;
};

/**
 * @brief Кватернион
 *
 */
using quaternion = vec4;
/**
 * @brief Создание кватерниона
 *
 * @param axis ось вращения
 * @param angle угол поворота вокруг оси
 * @return кватернион поворота
 */
quaternion make_quaternion(vec3 axis, double angle);
/**
 * @brief Параметры вращения
 *
 * @param q кватернион
 * @return  (ось вращения и угол поворота)
 */
rotation_desc from_quaternion(const quaternion& q);
/**
 * @brief Кватернион поворота между двумя векторами
 *
 * @param left левый
 * @param right правый
 * @return кватернион поворота
 */
quaternion rotation(vec3 left, vec3 right);
/**
 * @brief Сопряжённый кватернион
 *
 * @param q исходный кватернион
 */
constexpr quaternion conj(const quaternion& q) {
	return { q[0], -q[1], -q[2], -q[3] };
}
/**
 * @brief Нахождение обратного кватерниона
 *
 * @param q исходный кватернион
 * @return обратный кватернион
 */
constexpr quaternion inverse(const quaternion& q) {
	return conj(q) / (q * q);
}
/**
 * @brief Скалярное произведение кватернионов
 *
 * @param left левый
 * @param right правый
 * @return скалярное произведение
 */
constexpr double dot(const quaternion& left, const quaternion& right) {
	return left * right;
}
/**
 * @brief Произведение кватернионов
 *
 * @param left левый
 * @param right правый
 * @return кватернион
 */
constexpr quaternion mult(const quaternion& left, const quaternion& right) {
	auto le = slice<1, 3>(left);
	auto ri = slice<1, 3>(right);
	auto re = left[0] * ri + right[0] * le + cross(le, ri);
	return {
		left[0] * right[0] - le * ri,
		re[0], re[1], re[2]
	};
}
/**
 * @brief Вращение вектора с помощью кватерниона
 *
 * @param v исходный вектор
 * @param q кватернион вращения
 * @return вектор, полученный вращением
 */
constexpr vec3 rotate(const vec3& v, const quaternion& q) {
	quaternion qv;
	for (size_t i{}; i < v.size(); ++i) qv[1 + i] = v[i];
	return slice<1, 3>(mult(mult(q, qv), inverse(q)));
}
