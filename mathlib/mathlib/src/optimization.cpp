#include <optimization.hpp>
#include <cmath>
#include <vector>
#include <future>
#include <atomic>

namespace math
{

    iteration::iteration(iteration &&other) noexcept : n{other.n},
                                                       r{other.r},
                                                       v{std::move(other.v)},
                                                       dv{std::move(other.dv)},
                                                       rv{std::move(other.rv)}
    {
    }

    iteration &iteration::operator=(iteration &&other) noexcept
    {
        n = other.n;
        r = other.r;
        v = std::move(other.v);
        dv = std::move(other.dv);
        rv = std::move(other.rv);
        return *this;
    }

    static double residual_function(vector const &v)
    {
        return std::sqrt(v * v) / v.size();
    }

    static bool is_equal(double left, double right, double eps)
    {
        // абсолютное
        if (std::abs(left - right) < eps)
            return true;
        // относительное
        return std::abs(1 - right / left) < eps;
    }

    namespace
    {

        template <typename F>
        class invocable_range
        {
            F const &_func;
            std::atomic<std::size_t> _begin;
            std::size_t _end;

        public:
            invocable_range(F const &func, std::size_t begin, std::size_t end)
                : _func{func},
                  _begin{begin},
                  _end{end}
            {
            }
            bool invoke()
            {
                std::size_t curr = _begin++;
                if (curr < _end)
                {
                    _func(curr);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        };

        template <typename F>
        void execute(invocable_range<F> *inv)
        {
            while (inv->invoke())
                ;
        }

        template <typename F>
        void parallel_compute(std::size_t begin, std::size_t end, F const &func)
        {
            invocable_range<F> inv{func, begin, end};
            std::size_t cores_count = std::thread::hardware_concurrency();
            std::vector<std::future<void>> futures(std::min(cores_count, end - begin) - 1);
            for (auto &f : futures)
            {
                f = std::async(std::launch::async, &execute<F>, &inv);
            }
            execute(&inv);
            for (auto &f : futures)
            {
                f.wait();
            }
        }
    }

    /**
     * @brief Для формирования СЛАУ
     *
     */
    class equation_maker
    {
        /**
         * @brief Провайдер невязок с измерениями
         *
         */
        measurer const &_meas;
        /**
         * @brief Вектор вариаций параметров
         *
         */
        vector _dv;

    public:
        equation_maker(measurer const &meas, variator const &var) : _meas{meas}, _dv(var.get_variations())
        {
        }
        void operator()(vector const &v, matrix &mx, vector &rv) const
        {

            std::vector<vector> vectors(_dv.size() + 1);
            auto compute_func = [this, &vectors, &rv, &v](std::size_t i)
            {
                if (i < _dv.size())
                {
                    auto p{v};
                    p[i] += _dv[i];
                    vectors[i] = _meas.get_residuals(p);
                }
                else
                {
                    rv = _meas.get_residuals(v);
                }
            };
            parallel_compute({}, _dv.size() + 1, compute_func);
            if (mx.rows() != _dv.size() || mx.columns() != rv.size())
            {
                mx = matrix(_dv.size(), rv.size());
            }
            // матрица из частных производных
            for (std::size_t row{}; row < mx.rows(); ++row)
            {
                // невязки с учётом вариации вектора параметров
                auto &dv = vectors[row];
                for (std::size_t col{}; col < mx.columns(); ++col)
                {
                    mx[row][col] = (rv[col] - dv[col]) / _dv[row];
                }
            }
        }
    };

    std::size_t newton(vector &v, measurer const &meas, variator const &var, correlator const *cor, iterations_saver *handler, double eps, std::size_t maxiter)
    {
        equation_maker eqm{meas, var};
        vector rv;
        matrix dm;
        matrix cm;
        if (cor)
        {
            cm = cor->get_correlation();
        }
        double prev{};
        for (std::size_t i{1}; i < maxiter; ++i)
        {
            eqm(v, dm, rv);
            double curr = residual_function(rv);
            vector dv = lstsq(dm, rv, cor ? &cm : nullptr);
            bool stop = is_equal(curr, prev, eps);
            if (handler)
            {
                iteration iter;
                iter.n = i;
                iter.r = curr;
                iter.v = v;
                iter.rv = rv;
                if (!stop)
                {
                    iter.dv = dv;
                }
                handler->save(std::move(iter));
            }
            if (stop)
            {
                return i;
            }
            v += dv;
            prev = curr;
        }
        return maxiter;
    }

    /**
     * @brief Информация об оптимизации множителя
     *
     */
    struct optimize_info
    {
        /**
         * @brief Значение функции невязок
         *
         */
        double r;
        /**
         * @brief Вектор поправок
         *
         */
        vector dv;

        optimize_info() = default;
        optimize_info(optimize_info const &) = default;
        optimize_info(optimize_info &&other) noexcept : r{other.r}, dv{std::move(other.dv)} {}
        optimize_info &operator=(optimize_info const &) = default;
        optimize_info &operator=(optimize_info &&other) noexcept
        {
            r = other.r;
            dv = std::move(other.dv);
            return *this;
        }
    };

    /**
     * @brief Оптимизатор множителя
     *
     */
    class optimization_helper
    {
        measurer const &_meas;
        vector const &_v;
        vector _rv;
        matrix _sm;

    public:
        optimization_helper(measurer const &meas, vector const &v, matrix const &dm, vector const &rv, matrix const *cm)
            : _meas{meas}, _v{v}
        {
            _sm = dm * transpose(dm);
            if (cm)
            {
                _sm += *cm;
            }
            _rv = dm * rv;
        }
        optimize_info operator()(double mul) const
        {
            mul += 1;
            matrix sm{_sm};
            // добавляем к матрицы системы диагональную
            for (std::size_t i{}; i < sm.rows(); ++i)
            {
                sm[i][i] *= mul;
            }
            inverse(sm);
            optimize_info in;
            in.dv = sm * _rv;
            vector rv = _meas.get_residuals(_v + in.dv);
            in.r = residual_function(rv);
            return in;
        }
    };

    /**
     * @brief Оптимизация значения множителя.
     *
     * @param helper оптимизатор
     * @param resid исходная невязка
     * @param eps точность
     * @param maxiter максимальное кол-во итераций
     * @return optimize_info
     */
    optimize_info optimize_mult(optimization_helper const &helper, double resid, double eps, std::size_t maxiter)
    {
        double mult{0.2};
        optimize_info in{};
        in.r = resid;
        for (std::size_t i{1}; i <= maxiter; ++i)
        {
            optimize_info arr[3]{};
            double mularr[3]{0.5 * mult, mult, 1.5 * mult};
            auto func = [&arr, &mularr, &helper](std::size_t i)
            { arr[i] = helper(mularr[i]); };
            parallel_compute(std::size_t{}, std::size_t{3}, func);
            // условие, что вариация множителя вызывает ощутимое изменение невязки
            bool stop = is_equal(arr[0].r, arr[2].r, eps);
            stop |= arr[0].r > arr[1].r && arr[2].r > arr[1].r;
            // обновление
            if (arr[1].r < in.r)
            {
                in = arr[1];
            }
            // проверка останова
            if (stop)
            {
                return in;
            }
            // коррекция значения множителя
            double cor = (mularr[0] - mularr[2]) / (arr[0].r - arr[2].r);
            mult = cor > 0 ? mularr[0] : mularr[2];
        }
        return in;
    }

    void print(matrix const &mx, char const *);

    std::size_t levmarq(vector &v, measurer const &meas, variator const &var, correlator const *cor, iterations_saver *handler, double eps, std::size_t maxiter)
    {
        equation_maker eqm{meas, var};
        vector rv;
        matrix dm;
        matrix cm;
        if (cor)
        {
            cm = cor->get_correlation();
        }
        for (std::size_t i{1}; i < maxiter; ++i)
        {
            eqm(v, dm, rv);
            // print(dm, "matrix num.txt");
            double res = residual_function(rv);
            auto info = optimize_mult(optimization_helper{meas, v, dm, rv, cor ? &cm : nullptr}, res, eps, maxiter);
            bool stop = is_equal(res, info.r, eps);
            if (handler)
            {
                iteration iter;
                iter.n = i;
                iter.r = res;
                iter.v = v;
                iter.rv = rv;
                if (!stop)
                {
                    iter.dv = info.dv;
                }
                handler->save(std::move(iter));
            }
            if (stop)
            {
                return i;
            }
            v += info.dv;
        }
        return maxiter;
    }

    std::size_t levmarq(vector &v, residuals_provider const &prov, iterations_saver *handler, double eps, std::size_t maxiter)
    {
        vector rv;
        matrix dm;
        for (std::size_t i{1}; i < maxiter; ++i)
        {
            prov.get_residuals_and_derivatives(v, rv, dm);
            // print(dm, "matrix var.txt");
            double res = residual_function(rv);
            auto info = optimize_mult(optimization_helper{prov, v, dm, rv, nullptr}, res, eps, maxiter);
            bool stop = is_equal(res, info.r, eps);
            if (handler)
            {
                iteration iter;
                iter.n = i;
                iter.r = res;
                iter.v = v;
                iter.rv = rv;
                if (!stop)
                {
                    iter.dv = info.dv;
                }
                handler->save(std::move(iter));
            }
            if (stop)
            {
                return i;
            }
            v += info.dv;
        }
        return maxiter;
    }
}

#include <fstream>

namespace math
{
    void print(matrix const &mx, const char *filename)
    {
        std::ofstream fout(filename);
        for (size_t r{}; r < mx.rows(); ++r)
        {
            for (size_t c{}; c < mx.columns(); ++c)
            {
                fout << mx[r][c] << ' ';
            }
            fout << '\n';
        }
    }
}