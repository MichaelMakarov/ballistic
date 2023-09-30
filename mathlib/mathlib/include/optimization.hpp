#pragma once
#include <maths.hpp>

namespace math
{
    /**
     * @brief Интерфейс для предоставления информации об измерениях и невязке с вычислениями
     *
     */
    struct measurer
    {
        virtual ~measurer() = default;
        virtual vector get_residuals(vector const &v) const = 0;
    };
    /**
     * @brief Интерфейс для предостваления вариаций параметров
     *
     */
    struct variator
    {
        virtual ~variator() = default;
        virtual vector get_variations() const = 0;
    };
    /**
     * @brief Интерфейс для предоставления матрицы корреляции
     *
     */
    struct correlator
    {
        virtual ~correlator() = default;
        virtual matrix get_correlation() const = 0;
    };

    /**
     * @brief Итерация оптимизации
     *
     */
    struct iteration
    {
        /**
         * @brief Номер итерации
         *
         */
        std::size_t n;
        /**
         * @brief Значение функции невязок
         *
         */
        double r;
        /**
         * @brief Вектор параметров
         *
         */
        vector v;
        /**
         * @brief Вектор поправок
         *
         */
        vector dv;
        /**
         * @brief Вектор невязок
         *
         */
        vector rv;

        iteration() = default;
        iteration(iteration const &) = default;
        iteration(iteration &&other) noexcept;
        iteration &operator=(iteration const &) = default;
        iteration &operator=(iteration &&) noexcept;
    };

    /**
     * @brief Интерфейс добавления итерации оптимизации
     *
     */
    struct iterations_saver
    {
        virtual ~iterations_saver() = default;
        virtual void save(iteration &&) = 0;
    };

    /**
     * @brief Решение задачи МНК методом Ньютона.
     *
     * @param v исходные оптимизируемые параметры
     * @param meas интерфейс модели вычислений и измерений
     * @param var интерфейс вариаций оптимизируемых параметров
     * @param saver контейнер итераций оптимизации
     * @param eps относительная точность задаёт порог оптимизации
     * @param iterations максимальное кол-во итераций оптимизации
     * @return std::size_t кол-во итераций
     */
    std::size_t newton(vector &v, measurer const &meas, variator const &var, correlator const *cor, iterations_saver *saver = nullptr, double eps = 1e-3, std::size_t iterations = 20);

    /**
     * @brief Решение задачи МНК методом Левенберга-Марквардта.
     *
     * @param v исходные оптимизируемые параметры
     * @param meas интерфейс модели вычислений и измерений
     * @param var интерфейс вариаций оптимизируемых параметров
     * @param saver контейнер итераций оптимизации
     * @param eps относительная точность задаёт порог оптимизации
     * @param iterations максимальное кол-во итераций оптимизации
     * @return std::size_t кол-во итераций
     */
    std::size_t levmarq(vector &v, measurer const &meas, variator const &var, correlator const *cor, iterations_saver *saver = nullptr, double eps = 1e-3, std::size_t iterations = 20);

    class residuals_provider : public measurer
    {
    public:
        virtual ~residuals_provider() = default;
        virtual void get_residuals_and_derivatives(math::vector const &, math::vector &, math::matrix &) const = 0;
    };

    std::size_t levmarq(vector &v, residuals_provider const &prov, iterations_saver *saver = nullptr, double eps = 1e-3, std::size_t iterations = 20);
}