/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include <boost/sml.hpp>

namespace hj
{

/**
 * @brief Representing current state of the error handler FSM.
 */
enum class err_status
{
    idle,
    handling,
    failed,
    success,
    unknown
};

/**
 * @brief Exception thrown when the deferred event queue reaches its capacity limit.
 * 
 * @note This exception indicates an infrastructure overflow rather than a business logic error.
 *       It will propagate directly to the caller of `match()` and will NOT be caught by `on_exception`.
 */
class defer_queue_overflow : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

namespace detail
{

struct defer_queue_monitor
{
    std::size_t current_size{0};
};

// event
template <typename T = std::error_code>
struct err_event
{
    T                              ec;
    std::function<void(const T &)> cb;

    explicit err_event(T e, std::function<void(const T &)> cb_fn = nullptr)
        : ec(std::move(e))
        , cb(std::move(cb_fn))
    {
    }
};

struct pass
{
};
struct resolved
{
};
struct fail
{
};
struct abort
{
};
struct reset
{
};

// state
struct idle
{
};
struct handling
{
};
struct failed
{
};
struct success
{
};

template <typename TElement, std::size_t MaxCapacity = 64>
class bounded_defer_queue
{
  public:
    using container_type  = std::deque<TElement>;
    using value_type      = typename container_type::value_type;
    using size_type       = typename container_type::size_type;
    using iterator        = typename container_type::iterator;
    using const_iterator  = typename container_type::const_iterator;
    using reference       = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

    bounded_defer_queue() = default;

    explicit bounded_defer_queue(std::shared_ptr<defer_queue_monitor> monitor)
        : _monitor(std::move(monitor))
    {
    }

    template <typename TEvent>
    void push_back(TEvent &&event)
    {
        if constexpr(MaxCapacity > 0)
        {
            if(_queue.size() >= MaxCapacity)
            {
                throw defer_queue_overflow(
                    "Defer queue overflow limit reached.");
            }
        }
        _queue.push_back(std::forward<TEvent>(event));
        sync_size();
    }

    iterator       begin() noexcept { return _queue.begin(); }
    iterator       end() noexcept { return _queue.end(); }
    const_iterator begin() const noexcept { return _queue.begin(); }
    const_iterator end() const noexcept { return _queue.end(); }
    const_iterator cbegin() const noexcept { return _queue.cbegin(); }
    const_iterator cend() const noexcept { return _queue.cend(); }

    void pop_front()
    {
        _queue.pop_front();
        sync_size();
    }

    bool      empty() const noexcept { return _queue.empty(); }
    size_type size() const noexcept { return _queue.size(); }

    void clear() noexcept
    {
        _queue.clear();
        sync_size();
    }

    iterator erase(const_iterator pos)
    {
        auto it = _queue.erase(pos);
        sync_size();
        return it;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        auto it = _queue.erase(first, last);
        sync_size();
        return it;
    }

  private:
    void sync_size() const noexcept
    {
        if(_monitor)
        {
            _monitor->current_size = _queue.size();
        }
    }

    container_type                       _queue;
    std::shared_ptr<defer_queue_monitor> _monitor;
};

template <std::size_t MaxCapacity>
struct bounded_defer_queue_template
{
    template <typename TElement>
    using type = bounded_defer_queue<TElement, MaxCapacity>;
};

// boost::sml impl
template <typename T = std::error_code>
struct error_handler_impl
{
    using transition_cb = std::function<void(err_status src, err_status dst)>;
    using exception_cb  = std::function<void(const std::exception_ptr &)>;

    explicit error_handler_impl(transition_cb fn    = nullptr,
                                exception_cb  ex_fn = nullptr)
        : on_transition(std::move(fn))
        , on_exception(std::move(ex_fn))
    {
    }

    void safe_invoke(const std::function<void()> &fn) const noexcept
    {
        if(!fn)
            return;

        try
        {
            fn();
        }
        catch(...)
        {
            if(on_exception)
            {
                try
                {
                    on_exception(std::current_exception());
                }
                catch(...)
                {
                }
            }
        }
    }

    void safe_invoke_cb(const err_event<T> &e) const noexcept
    {
        if(e.cb)
        {
            safe_invoke([&] { e.cb(e.ec); });
        }
    }

    void safe_invoke_transition(err_status src, err_status dst) const noexcept
    {
        if(on_transition)
        {
            safe_invoke([&] { on_transition(src, dst); });
        }
    }

    auto operator()() const
    {
        using namespace boost::sml;

        return make_transition_table(
            // --- IDLE STATE ---
            *state<idle>
                + event<pass> /
                      [this] {
                          safe_invoke_transition(err_status::idle,
                                                 err_status::success);
                      } = state<success>,
            *state<idle>
                + event<fail> /
                      [this] {
                          safe_invoke_transition(err_status::idle,
                                                 err_status::failed);
                      } = state<failed>,
            *state<idle>
                + event<abort> /
                      [this] {
                          safe_invoke_transition(err_status::idle,
                                                 err_status::failed);
                      } = state<failed>,
            *state<idle>
                + event<err_event<T>> /
                      [this](const auto &e) {
                          safe_invoke_cb(e);
                          safe_invoke_transition(err_status::idle,
                                                 err_status::handling);
                      }                         = state<handling>,
            *state<idle> + event<reset> / [] {} = state<idle>,

            // --- SUCCESS STATE ---
            state<success> + event<pass> / [] {}     = state<success>,
            state<success> + event<resolved> / [] {} = state<success>,
            state<success> + event<abort> / [] {}    = state<success>,
            state<success>
                + event<err_event<T>> /
                      [this](const auto &e) {
                          safe_invoke_cb(e);
                          safe_invoke_transition(err_status::success,
                                                 err_status::handling);
                      } = state<handling>,
            state<success>
                + event<reset> /
                      [this] {
                          safe_invoke_transition(err_status::success,
                                                 err_status::idle);
                      } = state<idle>,

            // --- HANDLING STATE ---
            state<handling>
                + event<pass> /
                      [this] {
                          safe_invoke_transition(err_status::handling,
                                                 err_status::success);
                      } = state<success>,
            state<handling>
                + event<resolved> /
                      [this] {
                          safe_invoke_transition(err_status::handling,
                                                 err_status::success);
                      } = state<success>,
            state<handling>
                + event<fail> /
                      [this] {
                          safe_invoke_transition(err_status::handling,
                                                 err_status::failed);
                      } = state<failed>,
            state<handling>
                + event<abort> /
                      [this] {
                          safe_invoke_transition(err_status::handling,
                                                 err_status::failed);
                      } = state<failed>,
            state<handling>
                + event<reset> /
                      [this] {
                          safe_invoke_transition(err_status::handling,
                                                 err_status::idle);
                      } = state<idle>,
            state<handling> + event<err_event<T>> / defer,

            // --- FAILED STATE (Explicit Ignored Design) ---
            state<failed> + event<pass> / [] {}         = state<failed>,
            state<failed> + event<resolved> / [] {}     = state<failed>,
            state<failed> + event<fail> / [] {}         = state<failed>,
            state<failed> + event<abort> / [] {}        = state<failed>,
            state<failed> + event<err_event<T>> / [] {} = state<failed>,
            state<failed>
                + event<reset> /
                      [this] {
                          safe_invoke_transition(err_status::failed,
                                                 err_status::idle);
                      } = state<idle>);
    }

    transition_cb on_transition;
    exception_cb  on_exception;
};

} // namespace detail

/**
 * @brief Lightweight finite state machine wrapper for error handling and recovery.
 * 
 * @tparam T Error type, defaults to std::error_code.
 * @tparam IsOkFn Callable type determining if error represents success.
 * @tparam MaxDeferCapacity Capacity limit for deferred events (defaults to 64 for production safety).
 */
template <typename T                   = std::error_code,
          typename IsOkFn              = std::function<bool(const T &)>,
          std::size_t MaxDeferCapacity = 64>
class error_handler
{
    static_assert(
        std::is_invocable_r_v<bool, IsOkFn, const T &>,
        "IsOkFn must be a callable object with signature bool(const T &).");

  public:
    using is_ok_fn      = IsOkFn;
    using match_fn      = std::function<void(const T &)>;
    using transition_fn = std::function<void(err_status src, err_status dst)>;
    using exception_fn  = std::function<void(const std::exception_ptr &)>;

  private:
    using sm_impl      = detail::error_handler_impl<T>;
    using defer_policy = boost::sml::defer_queue<
        detail::bounded_defer_queue_template<MaxDeferCapacity>::template type>;
    using sm_type = boost::sml::sm<sm_impl, defer_policy>;

  public:
    error_handler()
        : _base{std::make_shared<sm_impl>(nullptr, nullptr)}
        , _monitor{std::make_shared<detail::defer_queue_monitor>()}
        , _sm{std::make_unique<sm_type>(*_base, _monitor)}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }

    static error_handler with_ok_value(T             ok_value,
                                       transition_fn on_transition = nullptr,
                                       exception_fn  on_exception  = nullptr)
    {
        return error_handler(
            [ok = std::move(ok_value)](const T &t) { return t == ok; },
            std::move(on_transition),
            std::move(on_exception));
    }

    static error_handler with_checker(is_ok_fn      is_ok,
                                      transition_fn on_transition = nullptr,
                                      exception_fn  on_exception  = nullptr)
    {
        return error_handler(std::move(is_ok),
                             std::move(on_transition),
                             std::move(on_exception));
    }

    static error_handler with_hooks(transition_fn on_transition,
                                    exception_fn  on_exception = nullptr)
    {
        return error_handler([](const T &t) { return !t; },
                             std::move(on_transition),
                             std::move(on_exception));
    }

    explicit error_handler(is_ok_fn      is_ok,
                           transition_fn on_transition = nullptr,
                           exception_fn  on_exception  = nullptr)
        : _base{std::make_shared<sm_impl>(std::move(on_transition),
                                          std::move(on_exception))}
        , _monitor{std::make_shared<detail::defer_queue_monitor>()}
        , _sm{std::make_unique<sm_type>(*_base, _monitor)}
        , _is_ok{std::move(is_ok)}
    {
    }

    explicit error_handler(is_ok_fn is_ok)
        : error_handler(std::move(is_ok), nullptr, nullptr)
    {
    }

    ~error_handler() noexcept = default;

    error_handler(const error_handler &)            = delete;
    error_handler &operator=(const error_handler &) = delete;

    error_handler(error_handler &&other) noexcept(
        std::is_nothrow_move_constructible_v<is_ok_fn>)
        : _base(std::move(other._base))
        , _monitor(std::move(other._monitor))
        , _sm(std::move(other._sm))
        , _is_ok(std::move(other._is_ok))
    {
    }

    error_handler &operator=(error_handler &&other) noexcept(
        std::is_nothrow_move_assignable_v<is_ok_fn>)
    {
        if(this != &other)
        {
            _base    = std::move(other._base);
            _monitor = std::move(other._monitor);
            _sm      = std::move(other._sm);
            _is_ok   = std::move(other._is_ok);
        }
        return *this;
    }

    /**
     * @brief Process an incoming error.
     * @throws hj::defer_queue_overflow if the handling queue exceeds MaxDeferCapacity.
     */
    error_handler &match(const T &err, match_fn cb = nullptr)
    {
        if(_is_ok(err))
        {
            _sm->process_event(detail::pass{});
        } else
        {
            _sm->process_event(detail::err_event<T>{err, std::move(cb)});
        }

        return *this;
    }

    error_handler &pass()
    {
        _sm->process_event(detail::pass{});
        return *this;
    }

    error_handler &resolve()
    {
        _sm->process_event(detail::resolved{});
        return *this;
    }

    error_handler &fail()
    {
        _sm->process_event(detail::fail{});
        return *this;
    }

    error_handler &abort()
    {
        _sm->process_event(detail::abort{});
        return *this;
    }

    error_handler &reset()
    {
        _monitor->current_size = 0;
        _sm                    = std::make_unique<sm_type>(*_base, _monitor);
        return *this;
    }

    [[nodiscard]] bool is_idle() const
    {
        return _sm->is(boost::sml::state<detail::idle>);
    }

    [[nodiscard]] bool is_handling() const
    {
        return _sm->is(boost::sml::state<detail::handling>);
    }

    [[nodiscard]] bool is_success() const
    {
        return _sm->is(boost::sml::state<detail::success>);
    }

    [[nodiscard]] bool is_failed() const
    {
        return _sm->is(boost::sml::state<detail::failed>);
    }

    [[nodiscard]] err_status status() const
    {
        if(_sm->is(boost::sml::state<detail::idle>))
            return err_status::idle;
        if(_sm->is(boost::sml::state<detail::handling>))
            return err_status::handling;
        if(_sm->is(boost::sml::state<detail::success>))
            return err_status::success;
        if(_sm->is(boost::sml::state<detail::failed>))
            return err_status::failed;

        return err_status::unknown;
    }

    [[nodiscard]] std::size_t deferred_size() const noexcept
    {
        return _monitor ? _monitor->current_size : 0;
    }

    [[nodiscard]] static constexpr std::size_t max_defer_capacity() noexcept
    {
        return MaxDeferCapacity;
    }

  private:
    std::shared_ptr<sm_impl>                     _base;
    std::shared_ptr<detail::defer_queue_monitor> _monitor;
    std::unique_ptr<sm_type>                     _sm;
    is_ok_fn                                     _is_ok;
};

} // namespace hj

#endif // ERROR_HANDLER_HPP