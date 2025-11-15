// This is a C++20 coroutine `task` implementation, based on that of cppcoro & libcoro.
// * https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/task.hpp
//   * Licensed under the MIT license, see `licenses/cppcoro.txt` for details.
// * https://github.com/jbaldwin/libcoro/blob/main/include/coro/task.hpp
//   * Licensed under the Apache-2.0 license, see `licenses/libcoro.txt` for details.
//
// Changes introduced by copyrat90 are:
// * Select between `lazy_task` or `eager_task` via template parameter `bool LazyStart`.
// * Custom stateless allocator support for promise types.
// * Get rid of exceptions, as exceptions are disabled in Butano by default.
// * Promise types are "private" by design.
//   * Add seperate `task::result()` getters because of this.
// * Simplify the code

#pragma once

#include <bn_assert.h>
#include <bn_optional.h>

#include <coroutine>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace ibn
{

namespace priv
{

template <bool LazyStart, typename Allocator>
class promise_base
{
public:
    static_assert(std::allocator_traits<Allocator>::is_always_equal::value, "Stateful allocator not supported");
    static_assert(sizeof(typename std::allocator_traits<Allocator>::value_type) == 1,
                  "Allocator is not byte allocator");

private:
    using initial_suspend_type = std::conditional_t<LazyStart, std::suspend_always, std::suspend_never>;

public:
    class final_awaiter
    {
    public:
        bool await_ready() const noexcept
        {
            return false;
        }

        template <typename Promise>
        auto await_suspend(std::coroutine_handle<Promise> handle) noexcept -> std::coroutine_handle<void>
        {
            auto continuation = handle.promise().continuation();
            if (continuation)
                return continuation;
            else
                return std::noop_coroutine();
        }

        void await_resume() noexcept
        {
        }
    };

public:
    promise_base() noexcept = default;

    promise_base(const promise_base&) noexcept = delete;
    promise_base& operator=(const promise_base&) noexcept = delete;

public:
    auto initial_suspend() noexcept -> initial_suspend_type
    {
        return {};
    }

    auto final_suspend() noexcept -> final_awaiter
    {
        return {};
    }

    void unhandled_exception() noexcept {};

public:
    void set_continuation(std::coroutine_handle<void> continuation)
    {
        _continuation = continuation;
    }

    auto continuation() const -> std::coroutine_handle<void>
    {
        return _continuation;
    }

public:
    void* operator new(std::size_t size)
    {
        return static_cast<void*>(Allocator{}.allocate(size));
    }

    void operator delete(void* ptr, std::size_t size)
    {
        Allocator{}.deallocate(reinterpret_cast<std::byte*>(ptr), size);
    }

private:
    std::coroutine_handle<void> _continuation;
};

template <typename T, bool LazyStart, typename Allocator>
class promise_t : public promise_base<LazyStart, Allocator>
{
public:
    template <typename U>
    void return_value(U&& value) noexcept
    {
        _result.emplace(std::forward<U>(value));
    }

    auto result() & noexcept -> decltype(auto)
    {
        return _result.value();
    }

    auto result() const& noexcept -> decltype(auto)
    {
        return _result.value();
    }

    auto result() && noexcept -> decltype(auto)
    {
        return std::move(_result.value());
    }

private:
    bn::optional<T> _result;
};

template <bool LazyStart, typename Allocator>
class promise_t<void, LazyStart, Allocator> : public promise_base<LazyStart, Allocator>
{
public:
    void return_void() noexcept {};

    void result() noexcept
    {
    }
};

template <typename T, bool LazyStart, typename Allocator>
class promise_t<T&, LazyStart, Allocator> : public promise_base<LazyStart, Allocator>
{
public:
    void return_value(T& value) noexcept
    {
        _result = std::addressof(value);
    }

    auto result() noexcept -> T&
    {
        return *_result;
    }

private:
    T* _result = nullptr;
};

} // namespace priv

template <typename T, bool LazyStart, typename Allocator = std::allocator<std::byte>>
class [[nodiscard]] task
{
public:
    class promise_type;

private:
    using handle_type = std::coroutine_handle<promise_type>;

    class awaiter_base
    {
    public:
        explicit awaiter_base(handle_type handle) noexcept : _handle(handle)
        {
        }

    public:
        bool await_ready() noexcept
        {
            return !_handle || _handle.done();
        }

        auto await_suspend(std::coroutine_handle<void> continuation) noexcept -> std::coroutine_handle<void>
        {
            _handle.promise().set_continuation(continuation);

            if constexpr (LazyStart)
                return _handle;
            else
                return std::noop_coroutine();
        }

    protected:
        handle_type _handle;
    };

public:
    class promise_type : public priv::promise_t<T, LazyStart, Allocator>
    {
    public:
        auto get_return_object() noexcept -> task
        {
            return task(handle_type::from_promise(*this));
        }
    };

public:
    ~task() noexcept
    {
        destroy();
    }

    /// @brief Default constructor.
    /// @note This creates a task that doesn't refer a coroutine.
    task() noexcept = default;

    /// @brief Move constructor.
    task(task&& other) noexcept : _handle(std::exchange(other._handle, nullptr))
    {
    }

    /// @brief Move assignment operator. (move-and-swap idiom)
    task& operator=(task other) noexcept
    {
        // Move and swap idiom
        swap(*this, other);

        return *this;
    }

    /// @brief Deleted copy constructor.
    task(const task&) noexcept = delete;

private:
    explicit task(handle_type handle) noexcept : _handle(handle)
    {
    }

public:
    /// @brief Checks if this task is completed, or the task doesn't refer a coroutine.
    bool done() const noexcept
    {
        return !_handle || _handle.done();
    }

    /// @brief Checks if this task refers to a valid coroutine.
    explicit operator bool() const noexcept
    {
        return _handle.operator bool();
    }

    /// @brief Resumes the execution of the referred coroutine.
    /// @note Errors out if this task doesn't refer a coroutine, or the task is already completed.
    /// (i.e. `done()` should return `false` to use this.)
    void resume() const noexcept
    {
        BN_BASIC_ASSERT(!done(), "task is done");

        _handle.resume();
    }

    /// @brief Resumes the execution of the referred coroutine.
    /// @note Errors out if this task doesn't refer a coroutine, or the task is already completed.
    /// (i.e. `done()` should return `false` to use this.)
    void operator()() const noexcept
    {
        BN_BASIC_ASSERT(!done(), "task is done");

        _handle();
    }

    /// @brief Destroys the coroutine state of the referred coroutine.
    /// After this, the task won't refer a coroutine.
    void destroy() noexcept
    {
        if (_handle)
        {
            _handle.destroy();
            _handle = nullptr;
        }
    }

public:
    /// @brief Gets the result from the promise of the referred coroutine.
    /// @note Errors out if this task doesn't refer a coroutine, or the task is not done yet.
    /// (i.e. `operator bool() && done()` should return `true` to use this.)
    auto result() & noexcept -> decltype(auto)
    {
        BN_BASIC_ASSERT(_handle, "task is not valid");
        BN_BASIC_ASSERT(_handle.done(), "task is not done");

        return promise().result();
    }

    /// @brief Gets the result from the promise of the referred coroutine.
    /// @note Errors out if this task doesn't refer a coroutine, or the task is not done yet.
    /// (i.e. `operator bool() && done()` should return `true` to use this.)
    auto result() const& noexcept -> decltype(auto)
    {
        BN_BASIC_ASSERT(_handle, "task is not valid");
        BN_BASIC_ASSERT(_handle.done(), "task is not done");

        return promise().result();
    }

    /// @brief Gets the result from the promise of the referred coroutine.
    /// @note Errors out if this task doesn't refer a coroutine, or the task is not done yet.
    /// (i.e. `operator bool() && done()` should return `true` to use this.)
    auto result() && noexcept -> decltype(auto)
    {
        BN_BASIC_ASSERT(_handle, "task is not valid");
        BN_BASIC_ASSERT(_handle.done(), "task is not done");

        return promise().result();
    }

public:
    /// @brief Gets an awaiter out of the task.
    auto operator co_await() const& noexcept
    {
        class awaiter : public awaiter_base
        {
        public:
            using awaiter_base::awaiter_base;

            auto await_resume() noexcept -> decltype(auto)
            {
                BN_BASIC_ASSERT(this->_handle, "task is not valid");

                return this->_handle.promise().result();
            }
        };

        return awaiter(_handle);
    }

    /// @brief Gets an awaiter out of the task.
    auto operator co_await() const&& noexcept
    {
        class awaiter : public awaiter_base
        {
        public:
            using awaiter_base::awaiter_base;

            auto await_resume() noexcept -> decltype(auto)
            {
                BN_BASIC_ASSERT(this->_handle, "task is not valid");

                return std::move(this->_handle.promise()).result();
            }
        };

        return awaiter(_handle);
    }

public:
    friend void swap(task& t1, task& t2) noexcept
    {
        using std::swap;
        swap(t1._handle, t2._handle);
    }

private:
    auto promise() & noexcept -> promise_type&
    {
        return _handle.promise();
    }

    auto promise() const& noexcept -> const promise_type&
    {
        return _handle.promise();
    }

    auto promise() && noexcept -> promise_type&&
    {
        return std::move(_handle.promise());
    }

private:
    handle_type _handle;
};

template <typename T, typename Allocator>
using lazy_task_t = task<T, true, Allocator>;

template <typename T, typename Allocator>
using eager_task_t = task<T, false, Allocator>;

template <typename T>
using lazy_task = lazy_task_t<T, std::allocator<std::byte>>;

template <typename T>
using eager_task = eager_task_t<T, std::allocator<std::byte>>;

} // namespace ibn
