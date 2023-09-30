#pragma once
#include <type_traits>
#include <initializer_list>

namespace math
{
    void throw_invalid_argument(char const *msg);
    void throw_runtime_error(char const *msg);
    void throw_out_of_range(char const *msg);

    /// Схема поворота углов ///
    enum RotationOrder
    {
        zyx = 0,
        zyz = 1,
        zxy = 2,
        zxz = 3,
        yxz = 4,
        yxy = 5,
        yzx = 6,
        yzy = 7,
        xyz = 8,
        xyx = 9,
        xzy = 10,
        xzx = 11
    };

    /// Математические константы ///

    /**
     * @brief Число ПИ
     */
    constexpr double pi{3.1415926535897932};
    /**
     * @brief Кол-во секунд в окружности
     *
     */
    constexpr double spr{1296000};

    /// Функции преобразования ///

    /**
     * @brief Преобразование градусов в радианы
     *
     * @param degrees градусы
     * @return радианы
     */
    inline constexpr double deg_to_rad(double degrees) noexcept
    {
        return degrees * (pi / 180);
    }
    /**
     * @brief Преобразование радиан в градусы
     *
     * @param radians радианы
     * @return градусы
     */
    inline constexpr double rad_to_deg(double radians) noexcept
    {
        return radians * (180 / pi);
    }
    /**
     * @brief Преобразование угловых секунд в радианы.
     *
     * @param seconds угловые секунды
     * @return constexpr double радианы
     */
    inline constexpr double sec_to_rad(double seconds) noexcept
    {
        return seconds * (pi / spr);
    }
    /**
     * @brief Преобразование радиан в угловые секунды.
     *
     * @param radians радианы
     * @return constexpr double угловые секунды
     */
    inline constexpr double rad_to_sec(double radians) noexcept
    {
        return radians * (spr / pi);
    }

    /**
     * @brief Интервал значений угла в рад
     */
    enum class round_type
    {
        /**
         * @brief [   0, 2pi    ]
         */
        zdpi,
        /**
         * @brief [ -pi, +pi    ]
         */
        mppi
    };

    /**
     * @brief Приведение угла в радианах к заданному диапазону значений.
     *
     * @tparam type тип диапазона
     * @param radians величина угла
     * @return double
     */
    template <round_type type = round_type::zdpi>
    double fit_round(double radians);

    namespace detail
    {
        double inverse(double *mx, std::size_t size);
        double sqrt(double);

        template <typename iterator>
        std::size_t distance(iterator begin, iterator end)
        {
            std::size_t count{};
            for (; begin != end; ++count, ++begin)
            {
            }
            return count;
        }

        template <typename T, bool = std::is_signed<T>::value>
        struct absolute
        {
            constexpr static T abs(T const &x)
            {
                return x < T() ? -x : x;
            }
        };
        template <typename T>
        struct absolute<T, false>
        {
            constexpr static T abs(T const &x)
            {
                return x;
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
    constexpr inline T cabs(const T &x) noexcept
    {
        return detail::absolute<T>::abs(x);
    }

    /**
     * @brief Сигнум числа
     *
     * @tparam T тип числа
     * @param x значение
     * @return знак числа
     */
    template <typename T>
    constexpr inline T sign(const T &x) noexcept
    {
        return (x > T()) - (x < T());
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
            if (t < begin)
                return begin;
            if (end < t)
                return end;
            return t;
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
     * @tparam iterator тип итератора
     * @param mean математическое ожидание
     * @param std среднеквадратическое отклонение
     * @param begin начальный итератор
     * @param end конечный итератор
     */
    template <typename iterator>
    void mean_std(double &mean, double &std, iterator begin, iterator end)
    {
        mean = std = 0;
        std::size_t count{};
        for (; begin != end; ++begin, ++count)
        {
            double tmp = *begin;
            mean += tmp;
            std += tmp * tmp;
        }
        mean /= count;
        std = detail::sqrt(std / count - mean * mean);
    }

    /// Вектор и матрица фиксированного размера ///

    /**
     * @brief Вектор фиксированного размера
     *
     * @tparam _size размер вектора
     */
    template <std::size_t _size>
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
                throw_invalid_argument("Кол-во элементов инициализации превышает размер вектора.");
            std::size_t i{};
            for (auto iter = list.begin(); iter != list.end(); ++iter)
                _elems[i++] = *iter;
        }
        vec &operator=(vec const &) = default;

        [[nodiscard]] constexpr double &operator[](std::size_t index) { return _elems[index]; }
        [[nodiscard]] constexpr const double &operator[](std::size_t index) const { return _elems[index]; }

        double *data() { return _elems; }
        const double *data() const { return _elems; }
        constexpr std::size_t size() const { return _size; }

        vec &operator+=(vec const &other)
        {
            for (std::size_t i{}; i < _size; ++i)
                _elems[i] += other._elems[i];
            return *this;
        }
        vec &operator-=(vec const &other)
        {
            for (std::size_t i{}; i < _size; ++i)
                _elems[i] -= other._elems[i];
            return *this;
        }
        vec &operator*=(double mul)
        {
            for (std::size_t i{}; i < _size; ++i)
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
            for (std::size_t i{}; i < _size; ++i)
                res._elems[i] = -v._elems[i];
            return res;
        }
        friend constexpr vec operator+(vec const &left, vec const &right)
        {
            vec result;
            for (std::size_t i{}; i < _size; ++i)
                result._elems[i] = left._elems[i] + right._elems[i];
            return result;
        }
        friend constexpr vec operator-(vec const &left, vec const &right)
        {
            vec result;
            for (std::size_t i{}; i < _size; ++i)
                result._elems[i] = left._elems[i] - right._elems[i];
            return result;
        }
        friend constexpr vec operator*(vec const &left, double right)
        {
            vec result;
            for (std::size_t i{}; i < _size; ++i)
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
            for (std::size_t i{}; i < _size; ++i)
                res += left._elems[i] * right._elems[i];
            return res;
        }

        /**
         * @brief Длина вектора
         *
         * @return double
         */
        double length() const { return detail::sqrt(sqr(*this)); }
        /**
         * @brief Нормирование вектора
         *
         */
        void normalize() { (*this) /= length(); }
        /**
         * @brief Возвращает вектор другого размера из элементов текущего вектора.
         *
         * @tparam begin начальный индекс, с которого нужно скопировать элементы текущего вектора
         * @tparam count кол-во элементов, которые нужно скопировать
         * @return constexpr vec<count>
         */
        template <std::size_t _begin, std::size_t _count>
        constexpr auto subv() const -> typename std::enable_if_t<_begin + _count <= _size, vec<_count>>
        {
            vec<_count> out;
            for (std::size_t i{}; i < _count; ++i)
                out[i] = _elems[_begin + i];
            return out;
        }
    };

    /**
     * @brief Матрица фиксированного размера
     *
     * @tparam _rows кол-во строк
     * @tparam _cols кол-во столбцов
     */
    template <std::size_t _rows, std::size_t _cols>
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
                throw_invalid_argument("Кол-во строк в списке инициализации превышает кол-во строк матрицы.");
            std::size_t r{};
            for (auto &row : list)
            {
                if (row.size() > _cols)
                    throw_invalid_argument("Кол-во элементов в строке списка инициализации превышает кол-во столбцов матрицы.");
                std::size_t c{};
                for (double el : row)
                    _elems[r][c++] = el;
                ++r;
            }
        }
        mat &operator=(mat const &) = default;

        [[nodiscard]] constexpr double *operator[](std::size_t index) { return _elems[index]; }
        [[nodiscard]] constexpr const double *operator[](std::size_t index) const { return _elems[index]; }

        using data_ptr = double (*)[_cols];
        using const_data_ptr = double const (*)[_cols];

        data_ptr data() { return _elems; }
        const_data_ptr data() const { return _elems; }

        constexpr std::size_t rows() const { return _rows; }
        constexpr std::size_t columns() const { return _cols; }

        mat &operator+=(mat const &other)
        {
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
                    _elems[r][c] += other._elems[r][c];
            return *this;
        }
        mat &operator-=(mat const &other)
        {
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
                    _elems[r][c] -= other._elems[r][c];
            return *this;
        }
        mat &operator*=(double mul)
        {
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
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
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
                    res._elems[r][c] = left._elems[r][c] + right._elems[r][c];
            return res;
        }
        friend constexpr mat operator-(mat const &left, mat const &right)
        {
            mat res;
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
                    res._elems[r][c] = left._elems[r][c] - right._elems[r][c];
            return res;
        }
        friend constexpr mat operator*(mat const &left, double right)
        {
            mat res;
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
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
            for (std::size_t r{}; r < _rows; ++r)
                for (std::size_t c{}; c < _cols; ++c)
                    res[r] += left[r][c] * right[c];
            return res;
        }
    };

    template <std::size_t _rows, std::size_t _dim, std::size_t _cols>
    constexpr mat<_rows, _cols> operator*(mat<_rows, _dim> const &left, mat<_dim, _cols> const &right)
    {
        mat<_rows, _cols> res;
        for (std::size_t r{}; r < _rows; ++r)
            for (std::size_t d{}; d < _dim; ++d)
                for (std::size_t c{}; c < _cols; ++c)
                    res[r][c] += left[r][d] * right[d][c];
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
    template <std::size_t _rows, std::size_t _cols>
    constexpr mat<_cols, _rows> transpose(const mat<_rows, _cols> &mx)
    {
        mat<_cols, _rows> tr;
        for (std::size_t r{}; r < _rows; ++r)
            for (std::size_t c{}; c < _cols; ++c)
                tr[c][r] = mx[r][c];
        return tr;
    }

    /**
     * @brief Вычисление определителя и обратной матрицы.
     *
     * @tparam dim размерность матрицы
     * @param mx исходная матрица (будет заполнена элементами обратной матрицы)
     * @return double определитель матрицы (если 0, то матрица вырождена)
     */
    template <std::size_t dim>
    double inverse(mat<dim, dim> &mx)
    {
        return detail::inverse(reinterpret_cast<double *>(mx.data()), dim);
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
     * @brief Расстояние между точкой и прямой, если значение > 0, то точка слева от прямой, иначе справа.
     */
    double distance(const line2d &line, const vec2 &point);
    /**
     * @brief Проекция точки на прямой.
     */
    vec2 projection(const line2d &line, const vec2 &point);

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
        /**
         * @brief Формирование кватерниона из самолётных углов.
         *
         * @param kren крен, угол вращения вокруг оси х (рад)
         * @param tang тангаж, угол вращения вокруг оси y (рад)
         * @param risk рысканье, угол вращения вокруг оси z (рад)
         * @return quaternion
         */
        static quaternion from_plane_angles(double kren, double tang, double risk);
        /**
         * @brief Преобразование кватерниона вращения в самолётные углы.
         *
         * @param q кватернион
         * @param kren крен, угол вращения вокруг оси х (рад)
         * @param tang тангаж, угол вращения вокруг оси y (рад)
         * @param risk рысканье, угол вращения вокруг оси z (рад)
         */
        static void to_plane_angles(quaternion const &q, double &kren, double &tang, double &risk);

        /**
         * @brief cos_matrix_to_euler Получает углы Эйлера (тангаж, крен, рысканье) на основании матрицы поворота(направляющих косинусов)
         * Возвращает углы по схеме 'ZYX'
         * Основано на функции matlab dcm2angle и схеме 'ZYX' X - крен; Y - тангаж; Z - рысканье
         * @param matrix - матрица поворота (направляющих косинусов)
         * @param risk
         * @param tang
         * @param kren
         */
        static void cos_matrix_to_euler(double *matrix, double &risk, double &tang, double &kren, RotationOrder rotation = RotationOrder::zyx);

        /**
         * @brief angles_to_quat Преобразование углов Эйлера в Кватернион
         * Основано на функции matlab angle2quat
         * По умолчанию используется порядок поворота по схеме 'ZYX' X - крен; Y - тангаж; Z - рысканье
         * @param risk - угол рысканья в радианах
         * @param tang - угол тангажа в радианах
         * @param kren - угол крена в радианах
         * @param quat - выходной кватеринион
         */
        static void angles_to_quat(double risk, double tang, double kren, quaternion &quat, RotationOrder rotation = RotationOrder::zyx);

        static void quat_to_angles(quaternion &quat, double &tang, double &kren, double &risk);

        /**
         * @brief Вычисление матрицы направляющих косинусов ССК -> ИСК
         * @param Кватернион
         * @param Матрица 3х3 направляющих косинусов (должна быть NULL)
         */
        static void quater_to_vsk_isk(quaternion &q, double *c);
        /**
         * @brief Получение кватерниона из матрицы поворотов по алгоритму Стенли (взято из комплекса EMK http://192.168.2.237/EMK/EMK.git )
         * @param Матрица 3*3 поворотов
         * @param Выходной кватернион на основе матрицы поворотов
         */
        static void quat_from_MatrixStanley(double *matr, quaternion &quat);
    };

    /**
     * @brief Вычисление косинуса угла между векторами.
     *
     * @tparam size размер векторов
     * @param left левый вектор
     * @param right правый вектор
     * @return double
     */
    template <std::size_t size>
    double cos_angle_of(vec<size> const &left, vec<size> const &right)
    {
        return (left * right) / detail::sqrt(sqr(left) * sqr(right));
    }

    /**
     * @brief Вектор динамической размерности
     */
    class vector
    {
        double *_elems;
        std::size_t _size;

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
        vector(std::size_t size);
        vector(std::size_t size, double value);
        vector(std::initializer_list<double> list);
        vector(const vector &other);
        vector(vector &&other) noexcept;
        ~vector();
        vector &operator=(const vector &other);
        vector &operator=(vector &&other) noexcept;

        std::size_t size() const noexcept { return _size; }
        double &operator[](std::size_t i)
        {
            return _elems[i];
        }
        const double &operator[](std::size_t i) const
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
        std::size_t _rows, _cols;

    private:
        void copy(const matrix &other);
        void move(matrix &other) noexcept;
        friend bool check_dimension(const matrix &left, const matrix &right) noexcept;

    public:
        matrix() noexcept;
        matrix(std::size_t rows, std::size_t columns);
        matrix(std::initializer_list<std::initializer_list<double>> list);
        matrix(const matrix &other);
        matrix(matrix &&other) noexcept;
        ~matrix();
        matrix &operator=(const matrix &other);
        matrix &operator=(matrix &&other) noexcept;

        std::size_t rows() const noexcept { return _rows; }
        std::size_t columns() const noexcept { return _cols; }

        double *data() noexcept { return _elems; }
        const double *data() const noexcept { return _elems; }

        double *operator[](std::size_t row)
        {
            return _elems + row * _cols;
        }
        const double *operator[](std::size_t row) const
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
     * @brief Вычисление определителя и обратной матрицы.
     *
     * @param mx исходная матрица (будет заполнена элементами обратной матрицы)
     * @return double определитель матрицы
     */
    double inverse(matrix &mx);
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
     * @param mx матрица системы размера nxm
     * @param vc вектор правой части размера mx1
     * @param cor указатель на матрицу корреляции размера nxn
     * @return vector вектор размера nx1
     */
    vector lstsq(const matrix &mx, const vector &vc, matrix const *cor = nullptr);

    /**
     * @brief Степенной полином
     *
     * @tparam _degree степень
     */
    template <std::size_t _degree>
    class polynomial
    {
        double _elems[_degree + 1];

    public:
        constexpr polynomial() noexcept : _elems{} {}
        constexpr polynomial(polynomial const &) noexcept = default;
        constexpr polynomial(std::initializer_list<double> list) : polynomial()
        {
            if (list.size() > _degree + 1)
                throw_invalid_argument("Кол-во значений инициализации полинома превышает необходимое.");
            std::size_t index{};
            for (double val : list)
                _elems[index++] = val;
        }
        polynomial &operator=(polynomial const &) noexcept = default;

        double *data() noexcept { return _elems; }
        const double *data() const noexcept { return _elems; }

        [[nodiscard]] constexpr double &operator[](std::size_t i) { return _elems[i]; }
        [[nodiscard]] constexpr const double &operator[](std::size_t i) const { return _elems[i]; }

        constexpr double operator()(double x) const noexcept
        {
            double mult{x}, total{_elems[0]};
            for (std::size_t i{1}; i <= _degree; ++i)
            {
                total += mult * _elems[i];
                mult *= x;
            }
            return total;
        }
    };

    namespace detail
    {
        template <typename F, typename T>
        struct is_convertible_to : std::is_convertible<std::remove_reference_t<F>, T>
        {
        };
    }

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
    template <std::size_t degree, typename iter_x, typename iter_y>
    polynomial<degree> polyfit(iter_x x, iter_y y, std::size_t count)
    {
        static_assert(detail::is_convertible_to<decltype(*x), double>::value, "Значения массива x должны быть double.");
        static_assert(detail::is_convertible_to<decltype(*x), double>::value, "Значения массива x должны быть double.");
        if (count <= degree)
            throw_invalid_argument("Кол-во точек должно быть больше степени полинома.");
        polynomial<degree> poly;
        matrix m(degree + 1, count);
        vector b(count);
        for (std::size_t i{}; i < count; ++i, ++x, ++y)
        {
            m[0][i] = 1.0;
            for (std::size_t k{1}; k < degree + 1; ++k)
                m[k][i] = m[k - 1][i] * (*x);
            b[i] = *y;
        }
        auto result = lstsq(m, b);
        for (std::size_t i{}; i < result.size(); ++i)
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
    template <std::size_t degree, typename iterator>
    polynomial<degree> polyfit(iterator begin, iterator end)
    {
        using first_value_type = decltype((*begin)[0]);
        using second_value_type = decltype((*begin)[1]);
        static_assert(detail::is_convertible_to<first_value_type, double>::value, "Значение по индексу 0 должно быть конвертируемо к double.");
        static_assert(detail::is_convertible_to<second_value_type, double>::value, "Значение по индексу 0 должно быть конвертируемо к double.");
        std::size_t count = detail::distance(begin, end);
        if (count <= degree)
            throw_invalid_argument("Кол-во точек должно быть больше степени полинома.");
        polynomial<degree> poly;
        matrix m(degree + 1, count);
        vector b(count);
        for (std::size_t i{}; begin != end; ++begin, ++i)
        {
            auto &point = *begin;
            m[0][i] = 1.0;
            for (std::size_t k{1}; k < degree + 1; ++k)
                m[k][i] = m[k - 1][i] * point[0];
            b[i] = point[1];
        }
        auto result = lstsq(m, b);
        for (std::size_t i{}; i < result.size(); ++i)
            poly[i] = result[i];
        return poly;
    }

#if __cplusplus >= 201703L
    template <typename T>
    interval(T, T) -> interval<T>;

    template <typename... R>
    vec(R...) -> vec<sizeof...(R)>;

    template <typename... T>
    polynomial(T...) -> polynomial<sizeof...(T) - 1>;
#endif
}
