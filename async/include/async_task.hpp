#pragma once

#include <coroutine>

template <typename R>
struct coroutine_promise;

template <typename R>
struct async_task
{
    using promise_type = coroutine_promise<R>;

    std::coroutine_handle<promise_type> _handle;
};

template <typename R>
struct basic_coroutine_promise
{
    std::suspend_never initial_suspend() const noexcept { return {}; }

    std::suspend_always final_suspend() const noexcept { return {}; }

    void unhandled_exception() {}

    async_task<R> get_return_object()
    {
        return {
            ._handle = std::coroutine_handle<coroutine_promise<R>>::from_promise(*reinterpret_cast<coroutine_promise<R> *>(this))};
    }
};

template <typename R>
struct coroutine_promise : basic_coroutine_promise<R>
{
    R _result;

    std::suspend_never yield_value(R &&value)
    {
        return return_value(std::move(value));
    }

    std::suspend_never return_value(R &&value)
    {
        _result = std::move(value);
        return {};
    }
};

template <>
struct coroutine_promise<void> : basic_coroutine_promise<void>
{
    void return_void() {}
};

template <typename R>
struct coroutine_awaiter
{
    std::coroutine_handle<coroutine_promise<R>> _handle;

    constexpr bool await_ready() const { return false; }

    void await_suspend(std::coroutine_handle<coroutine_promise<R>> h) { _handle = h; }

    constexpr void await_resume() const {}
};