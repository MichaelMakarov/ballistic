#pragma once
#include <mutex>
#include <iterator>

namespace par
{
    namespace detail
    {

        template <typename input, typename arg>
        struct is_convertible_to : std::is_convertible<input, arg>
        {
            static_assert(std::is_convertible<input, arg>::value, "Входной параметр невозможно преобразовать в типу аргумента функции.");
        };

        template <typename, typename>
        struct if_invocable;

        template <typename R, typename Arg, typename In>
        struct if_invocable<R (*)(Arg), In> : std::enable_if<is_convertible_to<In, Arg>::value>
        {
        };

        template <typename T, typename R, typename Arg, typename In>
        struct if_invocable<R (T::*)(Arg), In> : std::enable_if<is_convertible_to<In, Arg>::value>
        {
        };

        template <typename T, typename R, typename Arg, typename In>
        struct if_invocable<R (T::*)(Arg) const, In> : std::enable_if<is_convertible_to<In, Arg>::value>
        {
        };

        template <typename F, typename In>
        struct if_invocable : if_invocable<decltype(&F::operator()), In>
        {
        };

        template <typename I, bool = std::is_arithmetic<I>::value>
        struct retrieve_value
        {
            static auto value(I iter) -> decltype(*iter) { return *iter; }
            using type = decltype(value(I{}));
        };

        template <typename I>
        struct retrieve_value<I, true>
        {
            static I value(I index) { return index; }
            using type = I;
        };

        template <typename I>
        using retrieve_value_t = typename retrieve_value<I>::type;

        struct invocable
        {
            virtual bool invoke() = 0;
        };

        template <typename I, typename F>
        class invocable_range : public invocable
        {
            I _begin, _end;
            F &_func;
            std::mutex _sync;

        public:
            invocable_range(I begin, I end, F &func) : _begin{begin}, _end{end}, _func{func}
            {
            }
            bool invoke() override
            {
                I iter;
                {
                    std::lock_guard<std::mutex> lock{_sync};
                    if (_begin != _end)
                        iter = _begin++;
                    else
                        return false;
                }
                _func(retrieve_value<I>::value(iter));
                return true;
            }
        };

        template <typename I, bool = std::is_arithmetic<I>::value>
        struct distance_helper
        {
            static auto distance(I begin, I end) { return std::distance(begin, end); }
        };

        template <typename I>
        struct distance_helper<I, true>
        {
            static I distance(I begin, I end) { return end - begin; }
        };

        template <typename I>
        auto distance(I begin, I end)
        {
            return distance_helper<I>::distance(begin, end);
        }

        void parallel_for_impl(invocable &, size_t);
        void parallel_for_impl(invocable &, size_t, size_t);
    }

    /**
     * @brief Выполнение функции в многопоточном режиме.
     *
     * @tparam I тип входных данных (число либо итератор коллекции)
     * @tparam F тип выполняемой функции
     * @param begin начальный индекс / итератор
     * @param end конечный индекс / итератор (не входит)
     * @param func выполняемая функция (в качестве аргумента принимает число / значение разыменованного итератора )
     */
    template <typename I, typename F>
    auto parallel_for(I begin, I end, F &&func) -> typename detail::if_invocable<std::remove_reference_t<F>, detail::retrieve_value_t<I>>::type
    {
        auto count = detail::distance(begin, end);
        if (count > 0)
        {
            detail::invocable_range<I, F> range(begin, end, func);
            parallel_for_impl(range, static_cast<size_t>(count));
        }
    }

    /**
     * @brief Выполнение функции в многопоточном режиме.
     *
     * @tparam I тип входных данных (число либо итератор коллекции)
     * @tparam F тип выполняемой функции
     * @param begin начальный индекс / итератор
     * @param end конечный индекс / итератор (не входит)
     * @param func выполняемая функция (в качестве аргумента принимает число / значение разыменованного итератора )
     * @param threads кол-во потоков, в которых нужно выполнить
     */
    template <typename I, typename F>
    auto parallel_for(I begin, I end, F &&func, size_t threads) -> typename detail::if_invocable<std::remove_reference_t<F>, detail::retrieve_value_t<I>>::type
    {
        auto count = detail::distance(begin, end);
        if (count > 0 && threads > 0)
        {
            detail::invocable_range<I, F> range(begin, end, func);
            parallel_for_impl(range, static_cast<size_t>(count), threads);
        }
    }
}
