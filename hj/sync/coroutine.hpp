/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  withOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <boost/coroutine2/all.hpp>
#include <boost/context/detail/config.hpp>
#include <boost/context/stack_traits.hpp>
#include <exception>
#include <functional>
#include <utility>
#include <stdexcept>
#include <memory>
#include <cstddef>
#include <type_traits>
#include <string>

namespace hj
{

struct coroutine_options
{
    static std::size_t default_size() noexcept
    {
        return boost::context::stack_traits::default_size();
    }

    static std::size_t minimum_size() noexcept
    {
        return boost::context::stack_traits::minimum_size();
    }

    static std::size_t maximum_size() noexcept
    {
        return boost::context::stack_traits::maximum_size();
    }

    std::size_t stack_size = default_size();

    void validate() const
    {
        if(stack_size < minimum_size())
        {
            throw std::invalid_argument(
                "coroutine_options: stack_size (" + std::to_string(stack_size)
                + ") is smaller than minimum allowed size ("
                + std::to_string(minimum_size()) + ")");
        }

        if(boost::context::stack_traits::is_unbounded())
        {
            return;
        }

        if(stack_size > maximum_size())
        {
            throw std::invalid_argument("coroutine_options: stack_size ("
                                        + std::to_string(stack_size)
                                        + ") exceeds maximum allowed size ("
                                        + std::to_string(maximum_size()) + ")");
        }
    }
};

template <typename T = void>
class coroutine
{
  public:
    using boost_pull_type =
        typename boost::coroutines2::coroutine<T>::pull_type;
    using boost_push_type =
        typename boost::coroutines2::coroutine<T>::push_type;

    class yield_context
    {
      public:
        explicit yield_context(boost_push_type &sink)
            : _sink(sink)
        {
        }

        yield_context(const yield_context &)            = delete;
        yield_context &operator=(const yield_context &) = delete;
        yield_context(yield_context &&)                 = delete;
        yield_context &operator=(yield_context &&)      = delete;

        template <typename U,
                  typename = std::enable_if_t<!std::is_void_v<T>
                                              && std::is_convertible_v<U, T>>>
        void operator()(U &&value)
        {
            _sink(std::forward<U>(value));
        }

        template <typename Dummy = T,
                  typename       = std::enable_if_t<std::is_void_v<Dummy>>>
        void operator()()
        {
            _sink();
        }

      private:
        boost_push_type &_sink;
    };

    using coroutine_fn      = std::function<void(yield_context &)>;
    using exception_handler = std::function<void(const std::exception_ptr &)>;

  private:
    struct SharedState
    {
        std::exception_ptr exception_ptr{nullptr};
    };

  public:
    coroutine() noexcept = default;

    explicit coroutine(
        coroutine_fn      fn,
        exception_handler handler = [](const std::exception_ptr &) {})
        : coroutine(std::move(fn), coroutine_options{}, std::move(handler))
    {
    }

    explicit coroutine(
        coroutine_fn      fn,
        coroutine_options options,
        exception_handler handler = [](const std::exception_ptr &) {})
        : _state(std::make_shared<SharedState>())
    {
        options.validate();

        _pull = std::make_unique<boost_pull_type>(
            boost::context::fixedsize_stack(options.stack_size),
            [state = _state, fn = std::move(fn), handler = std::move(handler)](
                boost_push_type &sink) {
                try
                {
                    yield_context ctx(sink);
                    fn(ctx);
                }
                catch(const boost::context::detail::forced_unwind &)
                {
                    throw;
                }
                catch(...)
                {
                    state->exception_ptr = std::current_exception();
                    if(handler)
                    {
                        try
                        {
                            handler(state->exception_ptr);
                        }
                        catch(...)
                        {
                        }
                    }
                }
            });
    }

    coroutine(const coroutine &)            = delete;
    coroutine &operator=(const coroutine &) = delete;

    coroutine(coroutine &&other) noexcept
        : _state(std::move(other._state))
        , _pull(std::move(other._pull))
    {
    }

    coroutine &operator=(coroutine &&other) noexcept
    {
        if(this != &other)
        {
            _pull.reset();

            _state = std::move(other._state);
            _pull  = std::move(other._pull);
        }
        return *this;
    }

    ~coroutine() noexcept = default;

    void operator()()
    {
        if(_pull && *_pull)
        {
            (*_pull)();
        }
    }

    void drain() noexcept
    {
        if(_pull)
        {
            try
            {
                while(*_pull)
                {
                    (*_pull)();
                }
            }
            catch(...)
            {
            }
        }
    }

    template <typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
    T get() const
    {
        if(has_exception())
        {
            throw std::logic_error(
                "coroutine: cannot call get() after an unhandled exception was "
                "thrown inside coroutine.");
        }

        if(_pull && *_pull)
        {
            return (*_pull).get();
        }

        throw std::logic_error(
            "coroutine: cannot call get() on a completed coroutine without "
            "value.");
    }

    bool     operator!() const { return !is_resumable(); }
    explicit operator bool() const { return is_resumable(); }

    bool has_value() const noexcept
    {
        if constexpr(std::is_void_v<T>)
        {
            return false;
        } else
        {
            return (_pull && static_cast<bool>(*_pull));
        }
    }

    bool is_resumable() const noexcept
    {
        return _pull && static_cast<bool>(*_pull);
    }

    bool done() const noexcept { return !is_resumable(); }

    bool has_exception() const noexcept
    {
        return _state && _state->exception_ptr != nullptr;
    }

    std::exception_ptr exception() const noexcept
    {
        return _state ? _state->exception_ptr : nullptr;
    }

    void rethrow_exception() const
    {
        if(_state && _state->exception_ptr)
        {
            std::rethrow_exception(_state->exception_ptr);
        }
    }

  private:
    std::shared_ptr<SharedState>     _state;
    std::unique_ptr<boost_pull_type> _pull;
};

} // namespace hj

#define hj_coroutine_cat__(a, b) a##b
#define hj_coroutine_cat_(a, b) hj_coroutine_cat__(a, b)

#define HJ_COROUTINE(cmd)                                                      \
    ::hj::coroutine<void> hj_coroutine_cat_(__coroutine__, __COUNTER__)(       \
        [&](::hj::coroutine<void>::yield_context &) { cmd })

#endif // COROUTINE_HPP