#pragma once
#include <type_traits>
#include <cmath>
#include <stdexcept>
#include <numbers>

/// Функции преобразования ///

/**
 * @brief Преобразование градусов в радианы
 *
 * @param degrees градусы
 * @return радианы
 */
inline constexpr double deg_to_rad(double degrees) noexcept
{
    return degrees * (std::numbers::pi / 180);
}
/**
 * @brief Преобразование радиан в градусы
 *
 * @param radians радианы
 * @return градусы
 */
inline constexpr double rad_to_deg(double radians) noexcept
{
    return radians * (180 / std::numbers::pi);
}

/**
 * @brief Интервал значений угла в рад
 */
enum class round_type
{
    /**
     * @brief [   0, 2pi    ]
     */
    zero_double_pi,
    /**
     * @brief [ -pi, +pi    ]
     */
    minus_plus_pi
};

/**
 * @brief Приведение угла в радианах к заданному диапазону значений.
 *
 * @tparam type тип диапазона
 * @param radians величина угла
 * @return double
 */
template <round_type type = round_type::minus_plus_pi>
double fit_to_round(double radians);

template <>
double fit_to_round<round_type::zero_double_pi>(double radians);

template <>
double fit_to_round<round_type::minus_plus_pi>(double radians);

namespace internal
{

    template <bool>
    struct absolute
    {
        template <typename T>
        constexpr static T abs(const T &value)
        {
            return value >= T{} ? value : -value;
        }
    };
    template <>
    struct absolute<false>
    {
        template <typename T>
        constexpr static T abs(const T &value)
        {
            return value;
        }
    };

    template <bool>
    struct signum
    {
        template <typename T>
        constexpr static T sign(T const &value)
        {
            constexpr T zero{};
            if (value > zero)
                return T(1);
            else if (value < zero)
                return T(-1);
            return zero;
        }
    };

    template <>
    struct signum<false>
    {
        template <typename T>
        constexpr static T sign(T const &value)
        {
            constexpr T zero{};
            return value > zero ? T{1} : zero;
        }
    };
}

/// Арифметические функции ///

/**
 * @brief Возведение в квадрат
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return квадрат входного значения
 */
template <typename T>
constexpr inline auto sqr(const T &value)
{
    return value * value;
}
/**
 * @brief Возведение в куб
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return куб входного значения
 */
template <typename T>
constexpr inline auto cube(const T &value)
{
    return value * value * value;
}

/**
 * @brief Абсолютное значение
 *
 * @tparam T тип значения
 * @param value входное значение
 * @return абсолютное значения
 */
template <typename T>
constexpr inline T cabs(const T &value) noexcept
{
    return internal::absolute<std::is_signed<T>::value>::abs(value);
}
/**
 * @brief Сигнум числа
 *
 * @tparam T тип числа
 * @param value значение
 * @return знак числа
 */
template <typename T>
constexpr inline T sign(const T &value) noexcept
{
    return internal::signum<std::is_signed<T>::value>::sign(value);
}
/**
 * @brief Целочисленное деление с округлением вверх.
 *
 * @tparam T тип
 * @param divident делимое
 * @param divisor делитель
 * @return частное такое, что частное * делитель >= делимое
 */
template <typename T>
constexpr inline T divide_up(T divident, T divisor)
{
    static_assert(std::is_integral<T>::value, "Целочисленное деление с округлением предусмотрено только для целых чисел.");
    T quotient = divident / divisor;
    return (quotient * divisor == divident) ? quotient : quotient + T{1};
}

/// Статистика ///

/**
 * @brief Интервал значений
 * @tparam T тип значения (поддерживается сравнение на меньше-равно)
 */
template <typename T>
class interval
{
public:
    /**
     * @brief Начальная значение
     */
    T begin;
    /**
     * @brief Конечное значение
     */
    T end;
    /**
     * @brief Проверка, содержится ли значение на интервале.
     * @param t значение
     * @return true значение содержится в замкнутом интервале
     * @return false значение не содержится
     */
    constexpr bool operator()(const T &t) const noexcept
    {
        return begin <= t && t <= end;
    }
    /**
     * @brief Ближайшее значение на интервале.
     * @param t значение
     * @return значение на интервале
     */
    constexpr T nearest(const T &t) const noexcept
    {
        return std::max(begin, std::min(t, end));
    }
};

template <typename T>
constexpr interval<T> make_interval(const T &begin, const T &end)
{
    return interval<T>{begin, end};
}

/**
 * @brief Вычисление МО и СКО.
 *
 * @tparam fwdit итератор прямого последовательного перемещения по коллекции
 * @tparam F тип функции, принимающей элемент коллекции и возвращающей число
 * @param begin итератор на первый элемент
 * @param end итератор на конец
 * @param func функция извлечения требуемого параметра из элемента коллекции
 * @return статистические данные
 */
template <typename fwdit, typename F>
constexpr auto mean_std(fwdit begin, fwdit end, F &&func) -> std::pair<double, double>
{
    double mean{}, std{};
    size_t count{};
    for (; begin != end; ++begin, ++count)
    {
        double tmp = func(*begin);
        mean += tmp;
        std += tmp * tmp;
    }
    mean /= count;
    std = std::sqrt(std / count - mean * mean);
    return std::make_pair(mean, std);
}

/**
 * @brief Вычисление МО и СКО
 *
 * @tparam fwdit итератор последовательного доступа к колеекции чисел
 * @param begin итератор на начало коллекции
 * @param end итератор на конец коллекции
 * @return статистические данные
 */
template <typename fwdit>
constexpr auto mean_std(fwdit begin, fwdit end) -> std::pair<double, double>
{
    return mean_std(begin, end, [](const auto &x)
                    { return x; });
}

/// Вектор и матрица фиксированного размера ///

/**
 * @brief Вектор фиксированного размера
 *
 * @tparam _size размер вектора
 */
template <size_t _size>
class vec
{
    static_assert(_size > 0, "Нельзя создать вектор нулевой длины.");
    double _elems[_size];

public:
    constexpr vec() : _elems{} {}
    constexpr vec(vec const &) = default;
    constexpr vec(std::initializer_list<double> list) : vec()
    {
        if (list.size() > _size)
            throw std::invalid_argument("Кол-во элементов инициализации превышает размер вектора.");
        size_t i{};
        for (auto iter = list.begin(); iter != list.end(); ++iter)
            _elems[i++] = *iter;
    }
    vec &operator=(vec const &) = default;

    [[nodiscard]] constexpr double &operator[](size_t index) { return _elems[index]; }
    [[nodiscard]] constexpr const double &operator[](size_t index) const { return _elems[index]; }

    double *data() { return _elems; }
    const double *data() const { return _elems; }
    constexpr size_t size() const { return _size; }

    vec &operator+=(vec const &other)
    {
        for (size_t i{}; i < _size; ++i)
            _elems[i] += other._elems[i];
        return *this;
    }
    vec &operator-=(vec const &other)
    {
        for (size_t i{}; i < _size; ++i)
            _elems[i] -= other._elems[i];
        return *this;
    }
    vec &operator*=(double mul)
    {
        for (size_t i{}; i < _size; ++i)
            _elems[i] *= mul;
        return *this;
    }
    vec &operator/=(double div)
    {
        return vec::operator*=(1 / div);
    }

    friend constexpr vec operator-(vec const &v)
    {
        vec res;
        for (size_t i{}; i < _size; ++i)
            res._elems[i] = -v._elems[i];
        return res;
    }
    friend constexpr vec operator+(vec const &left, vec const &right)
    {
        vec result;
        for (size_t i{}; i < _size; ++i)
            result._elems[i] = left._elems[i] + right._elems[i];
        return result;
    }
    friend constexpr vec operator-(vec const &left, vec const &right)
    {
        vec result;
        for (size_t i{}; i < _size; ++i)
            result._elems[i] = left._elems[i] - right._elems[i];
        return result;
    }
    friend constexpr vec operator*(vec const &left, double right)
    {
        vec result;
        for (size_t i{}; i < _size; ++i)
            result._elems[i] = left._elems[i] * right;
        return result;
    }
    friend constexpr vec operator*(double right, vec const &left)
    {
        return left * right;
    }
    friend constexpr vec operator/(vec const &left, double right)
    {
        return left * (1 / right);
    }
    friend constexpr double operator*(vec const &left, vec const &right)
    {
        double res{};
        for (size_t i{}; i < _size; ++i)
            res += left._elems[i] * right._elems[i];
        return res;
    }

    /**
     * @brief Длина вектора
     *
     * @return double
     */
    double length() const { return std::sqrt(sqr(*this)); }
    /**
     * @brief Нормирование вектора
     *
     */
    void normalize() { (*this) /= length(); }
};

/**
 * @brief Матрица фиксированного размера
 *
 * @tparam _rows кол-во строк
 * @tparam _cols кол-во столбцов
 */
template <size_t _rows, size_t _cols>
class mat
{
    static_assert(_rows > 0, "Нельзя создать матрицу с 0 строками.");
    static_assert(_cols > 0, "Нельзя создать матрицу с 0 столбцами.");
    double _elems[_rows][_cols];

public:
    constexpr mat() : _elems{} {}
    constexpr mat(mat const &) = default;
    constexpr mat(std::initializer_list<std::initializer_list<double>> list) : mat()
    {
        if (list.size() > _rows)
            throw std::invalid_argument("Кол-во строк в списке инициализации превышает кол-во строк матрицы.");
        size_t r{};
        for (auto &row : list)
        {
            if (row.size() > _cols)
                throw std::invalid_argument("Кол-во элементов в строке списка инициализации превышает кол-во столбцов матрицы.");
            size_t c{};
            for (double el : row)
                _elems[r][c++] = el;
            ++r;
        }
    }
    mat &operator=(mat const &) = default;

    [[nodiscard]] constexpr double *operator[](size_t index) { return _elems[index]; }
    [[nodiscard]] constexpr const double *operator[](size_t index) const { return _elems[index]; }

    double *data() { return *_elems; }
    double const *data() const { return *_elems; }

    constexpr size_t rows() const { return _rows; }
    constexpr size_t columns() const { return _cols; }

    mat &operator+=(mat const &other)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] += other._elems[r][c];
        return *this;
    }
    mat &operator-=(mat const &other)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] -= other._elems[r][c];
        return *this;
    }
    mat &operator*=(double mul)
    {
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                _elems[r][c] *= mul;
        return *this;
    }
    mat &operator/=(double div)
    {
        return mat::operator*=(1 / div);
    }

    friend constexpr mat operator+(mat const &left, mat const &right)
    {
        mat res;
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                res._elems[r][c] = left._elems[r][c] + right._elems[r][c];
        return res;
    }
    friend constexpr mat operator-(mat const &left, mat const &right)
    {
        mat res;
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                res._elems[r][c] = left._elems[r][c] - right._elems[r][c];
        return res;
    }
    friend constexpr mat operator*(mat const &left, double right)
    {
        mat res;
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                res._elems[r][c] = left._elems[r][c] * right;
        return res;
    }
    friend constexpr mat operator*(double right, mat const &left)
    {
        return left * right;
    }
    friend constexpr mat operator/(mat const &left, double right)
    {
        return left * (1 / right);
    }
    friend constexpr vec<_rows> operator*(mat const &left, vec<_cols> const &right)
    {
        vec<_rows> res;
        for (size_t r{}; r < _rows; ++r)
            for (size_t c{}; c < _cols; ++c)
                res[r] += left[r][c] * right[c];
        return res;
    }
};

template <size_t _rows, size_t _cols, size_t _dim>
constexpr mat<_rows, _dim> operator*(mat<_rows, _cols> const &left, mat<_cols, _dim> const &right)
{
    mat<_rows, _dim> res;
    for (size_t r{}; r < _rows; ++r)
        for (size_t c{}; c < _cols; ++c)
            for (size_t d{}; d < _dim; ++d)
                res[r][d] += left[r][c] * right[c][d];
    return res;
}

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
using vec6 = vec<6>;
using mat2x2 = mat<2, 2>;
using mat3x3 = mat<3, 3>;
using mat4x4 = mat<4, 4>;

/**
 * @brief Извлечение подвектора/
 *
 * @tparam _beg нач. индекс
 * @tparam _end кон. индекс
 * @tparam _size размер исходного вектора
 * @param v исходный вектор
 * @return вектор vec<_end - _beg + 1>
 */
template <size_t _beg, size_t _end, size_t _size>
constexpr vec<_end - _beg + 1> subv(const vec<_size> &v)
{
    static_assert(_end < _size, "кон. индекс должен быть меньше размера вектора");
    static_assert(_beg < _end, "начю индекс должен быть меньше кон. индекса");
    vec<_end - _beg + 1> r;
    for (size_t i{_beg}, j{}; i <= _end; ++i, ++j)
    {
        r[j] = v[i];
    }
    return r;
}
/**
 * @brief Скалярное произведение двух векторов
 *
 * @param left левый
 * @param right правый
 * @return результирующий вектор
 */
constexpr vec3 cross(const vec3 &left, const vec3 &right)
{
    vec3 out;
    out[0] = left[1] * right[2] - left[2] * right[1];
    out[1] = left[2] * right[0] - left[0] * right[2];
    out[2] = left[0] * right[1] - left[1] * right[0];
    return out;
}
/**
 * @brief Построение матрицы перехода в систему координат по заданному вектору
 *
 * @param v вектор, задающий направление для первого базисного вектора
 * @return mat3x3 матрица перехода
 */
mat3x3 make_transform(vec3 v);
/**
 * @brief Транспонирование матрицы.
 *
 * @tparam _rows кол-во строк
 * @tparam _cols кол-во столбцов
 * @param mx исходная матрица
 * @return транспонированная матрица
 */
template <size_t _rows, size_t _cols>
constexpr mat<_cols, _rows> transpose(const mat<_rows, _cols> &mx)
{
    mat<_cols, _rows> tr;
    for (size_t r{}; r < _rows; ++r)
        for (size_t c{}; c < _cols; ++c)
            tr[c][r] = mx[r][c];
    return tr;
}
/**
 * @brief Вычисление обратной матрицы.
 *
 * @tparam dim размерность матрицы
 * @param mx исходная матрица
 * @return обратная матрица
 */
template <size_t dim>
constexpr mat<dim, dim> inverse(const mat<dim, dim> &mx)
{
    auto inv{mx};
    double pivot{};
    size_t descent[dim]{};
    size_t k{};
    for (size_t r = 0; r < dim; ++r)
        descent[r] = r;
    for (size_t i = 0; i < dim; ++i)
    {
        k = descent[i];
        pivot = inv[k][k];
        if (cabs(pivot) < 1e-20)
        {
            if (i == dim - 1)
            {
                throw std::invalid_argument("Матрица является вырожденной и не может быть обращена.");
            }
            std::swap(descent[i], descent[i + 1]);
            k = descent[i];
            pivot = inv[k][k];
        }
        for (size_t r = 0; r < dim; ++r)
            inv[r][k] /= -pivot;
        for (size_t r = 0; r < dim; ++r)
        {
            if (r != k)
            {
                for (size_t c = 0; c < dim; ++c)
                {
                    if (c != k)
                        inv[r][c] += inv[k][c] * inv[r][k];
                }
            }
        }
        for (size_t c = 0; c < dim; ++c)
            inv[k][c] /= pivot;
        inv[k][k] = 1 / pivot;
    }
    return inv;
}

/**
 * @brief Прямая на плоскости
 */
struct line2d
{
    /**
     * @brief направляющий вектор
     */
    vec2 direction;
    /**
     * @brief точка, через которую проодит прямая
     */
    vec2 point;
};
/**
 * @brief Расстояние между точкой и прямой, если значение > 0, то точка слева от прямой, иначе справа/
 */
double distance(const line2d &line, const vec2 &point);
/**
 * @brief Проекция точки на прямой.
 */
vec2 projection(const line2d &line, const vec2 &point);

/**
 * @brief Прямоугоьный фрагмент
 *
 * @tparam T тип координаты вершины
 */
template <typename T>
class rectangle
{
    template <bool>
    class helper
    {
        template <typename T>
        constexpr static T size(T const &begin, T const &end) { return end - begin; }
    };
    template <>
    class helper<true>
    {
        template <typename T>
        constexpr static T size(T const &begin, T const &end) { return end - begin + T{1}; }
    };

public:
    T begin_x;
    T begin_y;
    T end_x;
    T end_y;

    constexpr T width() const { return helper<std::is_integral<T>::value>::size(begin_x, end_x); }
    constexpr T height() const { return helper<std::is_integral<T>::value>::size(begin_y, end_y); }
    constexpr T square() const { return width() * height(); }
};

template <typename T>
constexpr rectangle<T> make_rectangle(const T &begin_x, const T &begin_y, const T &end_x, const T &end_y)
{
    return rectangle<T>{begin_x, begin_y, end_x, end_y};
}

/**
 * @brief Кватернион
 *
 */
class quaternion
{
    vec3 _v;
    double _s{1};

public:
    constexpr quaternion() = default;
    /**
     * @brief Инициализация кватерниона значениями.
     *
     */
    constexpr quaternion(double x, double y, double z, double s) : _v{x, y, z}, _s{s} {}
    /**
     * @brief Создание кватерниона вращения.
     *
     * @param axis ось вращения
     * @param angle угол вращения в рад
     */
    quaternion(vec3 const &axis, double angle);
    /**
     * @brief Создание кватерниона вращения как преобразование между двумя векторами.
     *
     * @param left левый вектор
     * @param right правый вектор, полученный вращением левого
     */
    quaternion(vec3 const &left, vec3 const &right);
    quaternion(quaternion const &) noexcept = default;
    quaternion &operator=(quaternion const &) noexcept = default;

    constexpr double s() const { return _s; }
    constexpr double x() const { return _v[0]; }
    constexpr double y() const { return _v[1]; }
    constexpr double z() const { return _v[2]; }

    /**
     * @brief Норма кватерниона
     *
     * @return double
     */
    double norm() const;
    /**
     * @brief Нормирование кватерниона
     *
     */
    void normalize();

    quaternion &operator+=(quaternion const &other)
    {
        _s += other._s;
        _v += other._v;
        return *this;
    }
    quaternion &operator-=(quaternion const &other)
    {
        _s -= other._s;
        _v -= other._v;
        return *this;
    }
    quaternion &operator*=(double val)
    {
        _s *= val;
        _v *= val;
        return *this;
    }
    quaternion &operator/=(double val)
    {
        return quaternion::operator*=(1 / val);
    }

    friend constexpr quaternion operator+(quaternion const &left, quaternion const &right)
    {
        quaternion res;
        res._s = left._s + right._s;
        res._v = left._v + right._v;
        return res;
    }
    friend constexpr quaternion operator-(quaternion const &left, quaternion const &right)
    {
        quaternion res;
        res._s = left._s - right._s;
        res._v = left._v - right._v;
        return res;
    }
    friend constexpr quaternion operator*(quaternion const &left, double right)
    {
        quaternion res;
        res._s = left._s * right;
        res._v = left._v * right;
        return res;
    }
    friend constexpr quaternion operator*(double right, quaternion const &left)
    {
        return left * right;
    }
    friend constexpr quaternion operator/(quaternion const &left, double right)
    {
        return left * (1 / right);
    }
    /**
     * @brief Скалярное произведение.
     *
     * @param left
     * @param right
     * @return constexpr double
     */
    friend constexpr double dot(quaternion const &left, quaternion const &right)
    {
        return left._s * right._s + left._v * right._v;
    }
    /**
     * @brief Перемножение кватернионов.
     *
     * @param left
     * @param right
     * @return constexpr quaternion
     */
    friend constexpr quaternion mul(quaternion const &left, quaternion const &right)
    {
        quaternion res;
        res._s = left._s * right._s - left._v * right._v;
        res._v = left._v * right._s + right._v * left._s + cross(left._v, right._v);
        return res;
    }

    /**
     * @brief Сопряженный кватернион.
     *
     * @param q исходный кватернион
     * @return constexpr quaternion
     */
    friend constexpr quaternion conj(quaternion const &q) noexcept
    {
        return quaternion(-q._v[0], -q._v[1], -q._v[2], q._s);
    }
    /**
     * @brief Обратный кватернион.
     *
     * @param q исходный кватернион
     * @return constexpr quaternion
     */
    friend constexpr quaternion inverse(quaternion const &q)
    {
        return conj(q) / dot(q, q);
    }
    /**
     * @brief Вращение вектора.
     *
     * @param v исходный вектор
     * @return vec3
     */
    constexpr vec3 rotate(vec3 const &v) const
    {
        auto &q = *this;
        return mul(mul(q, quaternion(v[0], v[1], v[2], 0)), inverse(q))._v;
    }
};

/**
 * @brief Вычисление косинуса угла между векторами.
 *
 * @tparam size размер векторов
 * @param left левый вектор
 * @param right правый вектор
 * @return double
 */
template <size_t size>
double cos_angle_of(vec<size> const &left, vec<size> const &right)
{
    return (left * right) / std::sqrt(sqr(left) + sqr(right));
}

/**
 * @brief Вектор динамической размерности
 */
class vector
{
    double *_elems;
    size_t _size;

private:
    void copy(const vector &other);
    void move(vector &other) noexcept;

public:
    using value_type = double;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = value_type *;
    using const_iterator = const value_type *;

public:
    vector() noexcept;
    vector(size_t size);
    vector(size_t size, double value);
    vector(std::initializer_list<double> list);
    vector(const vector &other);
    vector(vector &&other) noexcept;
    ~vector();
    vector &operator=(const vector &other);
    vector &operator=(vector &&other) noexcept;

    size_t size() const noexcept { return _size; }
    double &operator[](size_t i)
    {
        return _elems[i];
    }
    const double &operator[](size_t i) const
    {
        return _elems[i];
    }
    double *data() { return _elems; }
    const double *data() const { return _elems; }

    double &first();
    const double &first() const;
    double &last();
    const double &last() const;

    double *begin();
    const double *begin() const;
    double *end();
    const double *end() const;
    /**
     * @brief Длина вектора.
     *
     * @return double
     */
    double length() const;
    /**
     * @brief Нормирование вектора
     *
     */
    void normalize();

    vector &operator+=(const vector &other);
    vector &operator-=(const vector &other);
    vector &operator*=(double value);
    vector &operator/=(double value);

    friend vector operator+(const vector &left, const vector &right);
    friend vector operator-(const vector &left, const vector &right);
    friend double operator*(const vector &left, const vector &right);
    friend vector operator*(double value, const vector &vec);
    friend vector operator*(const vector &vec, double value);
    friend vector operator/(const vector &vec, double value);
};

/**
 * @brief Матрица динамической размерности
 */
class matrix
{
private:
    double *_elems;
    size_t _rows, _cols;

private:
    void copy(const matrix &other);
    void move(matrix &other) noexcept;
    friend bool check_dimension(const matrix &left, const matrix &right) noexcept;

public:
    matrix() noexcept;
    matrix(size_t rows, size_t columns);
    matrix(std::initializer_list<std::initializer_list<double>> list);
    matrix(const matrix &other);
    matrix(matrix &&other) noexcept;
    ~matrix();
    matrix &operator=(const matrix &other);
    matrix &operator=(matrix &&other) noexcept;

    size_t rows() const noexcept { return _rows; }
    size_t columns() const noexcept { return _cols; }

    double *data() noexcept { return _elems; }
    const double *data() const noexcept { return _elems; }

    double *operator[](size_t row)
    {
        return _elems + row * _cols;
    }
    const double *operator[](size_t row) const
    {
        return _elems + row * _cols;
    }

    matrix &operator+=(const matrix &other);
    matrix &operator-=(const matrix &other);
    matrix &operator*=(double v);
    matrix &operator/=(double v);

    friend matrix operator+(const matrix &left, const matrix &right);
    friend matrix operator-(const matrix &left, const matrix &right);
    friend matrix operator*(const matrix &left, const matrix &right);
    friend vector operator*(const matrix &mx, const vector &vec);
    friend matrix operator/(const matrix &mx, double value);
    friend matrix operator*(const matrix &mx, double value);
    friend matrix operator*(double value, const matrix &mx);
};

/**
 * @brief Вычисление транспонированной матрицы
 *
 * @param m исходная матрица
 * @return транспонированная матрица
 */
matrix transpose(const matrix &m);
/**
 * @brief Вычисление обратной матрицы
 *
 * @param mx исходная матрица
 * @param det определитель матрицы
 */
void inverse(matrix &mx, double *det = nullptr);
/**
 * @brief Умножение матрицы на диагональную справа
 *
 * @param mx исходная матрица
 * @param diag диагональная матрица
 */
void mxd(matrix &mx, const vector &diag);
/**
 * @brief Умножение матрицы на диагональную слева
 *
 * @param diag диагональная матрица
 * @param mx исходная матрица
 */
void dxm(const vector &diag, matrix &mx);
/**
 * @brief Решение задачи МНК
 *
 * @param mx матрица системы nxm
 * @param vc вектор правой части mx1
 * @return решение - вектор nx1
 */
vector lstsq(const matrix &mx, const vector &vc);

/**
 * @brief Степенной полином
 *
 * @tparam _degree степень
 */
template <size_t _degree>
class polynomial
{
    double _elems[_degree + 1];

public:
    constexpr polynomial() noexcept : _elems{} {}
    constexpr polynomial(polynomial const &) noexcept = default;
    constexpr polynomial(std::initializer_list<double> list) : polynomial()
    {
        if (list.size() > _degree + 1)
            throw std::invalid_argument("Кол-во значений инициализации полинома превышает необходимое.");
        size_t index{};
        for (double val : list)
            _elems[index++] = val;
    }
    polynomial &operator=(polynomial const &) noexcept = default;

    double *data() noexcept { return _elems; }
    const double *data() const noexcept { return _elems; }

    [[nodiscard]] constexpr double &operator[](size_t i) { return _elems[i]; }
    [[nodiscard]] constexpr const double &operator[](size_t i) const { return _elems[i]; }

    constexpr double operator()(double x) const noexcept
    {
        double mult{x}, total{_elems[0]};
        for (size_t i{1}; i <= _degree; ++i)
        {
            total += mult * _elems[i];
            mult *= x;
        }
        return total;
    }
};

/**
 * @brief Аппроксимация полиномом.
 *
 * @tparam degree степень полинома
 * @tparam iter_x итератор по x
 * @tparam iter_y итератор по y
 * @param x итератор по коллекции с координатами x
 * @param y итератор по коллекции с координатами y
 * @param count кол-во точек
 * @return polynomial<degree>
 */
template <size_t degree, typename iter_x, typename iter_y>
polynomial<degree> polyfit(iter_x x, iter_y y, size_t count)
{
    static_assert(std::is_same<std::remove_const_t<std::remove_reference_t<decltype(*x)>>, double>::value, "Значения массива x должны быть double.");
    static_assert(std::is_same<std::remove_const_t<std::remove_reference_t<decltype(*y)>>, double>::value, "Значения массива x должны быть double.");

    polynomial<degree> poly;
    matrix m(degree + 1, count);
    vector b(count);
    for (size_t i{}; i < count; ++i, ++x, ++y)
    {
        m[0][i] = 1.0;
        for (size_t k{1}; k < degree + 1; ++k)
            m[k][i] = m[k - 1][i] * (*x);
        b[i] = *y;
    }
    auto result = lstsq(m, b);
    for (size_t i{}; i < result.size(); ++i)
        poly[i] = result[i];
    return poly;
}
/**
 * @brief Аппроксимация полиномом массива точек
 *
 * @tparam degree степенб полинома
 * @tparam iter итератор последовательного доступа
 * @param begin итератор на начало массива точек
 * @param end итератор на конец массива точек
 * @return аппроксимирующий полином
 */
template <size_t degree, typename iter>
polynomial<degree> polyfit(iter begin, iter end)
{
    static_assert(std::is_same<std::remove_reference_t<decltype(*begin)>, vec2>::value, "Значения массива должны быть векторами vec2.");
    size_t count = std::distance(begin, end);
    polynomial<degree> poly;
    matrix m(degree + 1, count);
    vector b(count);
    for (size_t i{}; begin != end; ++begin, ++i)
    {
        const vec2 &point = *begin;
        m[0][i] = 1.0;
        for (size_t k{1}; k < degree + 1; ++k)
            m[k][i] = m[k - 1][i] * point[0];
        b[i] = point[1];
    }
    auto result = lstsq(m, b);
    for (size_t i{}; i < result.size(); ++i)
        poly[i] = result[i];
    return poly;
}

#if __cplusplus >= 201703L
template <typename T>
interval(T, T) -> interval<T>;

template <typename T>
rectangle(T, T, T, T) -> rectangle<T>;

template <typename... R>
vec(R...) -> vec<sizeof...(R)>;

template <typename... T>
polynomial(T...) -> polynomial<sizeof...(T) - 1>;
#endif
