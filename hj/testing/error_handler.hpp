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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>
#include <memory>
#include <functional>
#include <system_error>
#include <deque>

#include <boost/sml.hpp>

namespace hj
{
namespace detail
{

// event
template <typename T = std::error_code>
struct err_event
{
    T                              ec;
    std::function<void(const T &)> cb;

    explicit err_event(T e, std::function<void(const T &)> &&cb)
        : ec(e)
        , cb(std::move(cb))
    {
    }
};
struct finish
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
struct succed
{
};

// boost::sml
template <typename T = std::error_code>
struct error_handler_impl
{
    error_handler_impl(
        std::function<void(const char *src, const char *dst)> &&fn)
        : on_transition(std::move(fn))
    {
    }

    auto operator()() const
    {
        return boost::sml::make_transition_table(
            *boost::sml::state<idle>
                + boost::sml::event<finish>
                      / [this]() { on_transition("idle", "succed"); } =
                boost::sml::state<succed>,
            *boost::sml::state<idle>
                + boost::sml::event<fail>
                      / [this]() { on_transition("idle", "failed"); } =
                boost::sml::state<failed>,
            *boost::sml::state<idle>
                + boost::sml::event<abort>
                      / [this]() { on_transition("idle", "failed"); } =
                boost::sml::state<failed>,
            *boost::sml::state<idle>
                + boost::sml::event<err_event<T>> /
                      [this](const auto &e) {
                          e.cb(e.ec);
                          on_transition("idle", "handling");
                      } = boost::sml::state<handling>,
            *boost::sml::state<idle> + boost::sml::event<reset> / [this]() {} =
                boost::sml::state<idle>,

            boost::sml::state<succed>
                + boost::sml::event<finish> / [this]() {} =
                boost::sml::state<succed>,
            boost::sml::state<succed> + boost::sml::event<abort> / [this]() {} =
                boost::sml::state<succed>,
            boost::sml::state<succed>
                + boost::sml::event<err_event<T>> /
                      [this](const auto &e) {
                          e.cb(e.ec);
                          on_transition("succed", "handling");
                      } = boost::sml::state<handling>,
            boost::sml::state<succed>
                + boost::sml::event<reset>
                      / [this]() { on_transition("succed", "idle"); } =
                boost::sml::state<idle>,

            boost::sml::state<handling>
                + boost::sml::event<finish>
                      / [this]() { on_transition("handling", "succed"); } =
                boost::sml::state<succed>,
            boost::sml::state<handling>
                + boost::sml::event<fail>
                      / [this]() { on_transition("handling", "failed"); } =
                boost::sml::state<failed>,
            boost::sml::state<handling>
                + boost::sml::event<reset>
                      / [this]() { on_transition("handling", "idle"); } =
                boost::sml::state<idle>,
            boost::sml::state<handling>
                + boost::sml::event<abort>
                      / [this]() { on_transition("handling", "failed"); } =
                boost::sml::state<failed>,
            boost::sml::state<handling>
                + boost::sml::event<err_event<T>> / boost::sml::defer,

            boost::sml::state<failed>
                + boost::sml::event<reset>
                      / [this]() { on_transition("failed", "idle"); } =
                boost::sml::state<idle>);
    }

    std::function<void(const char *src, const char *dst)> on_transition;
};

} // namespace detail

enum class err_status
{
    idle,
    handling,
    failed,
    succed,
    unknown
};

template <typename T = std::error_code>
class error_handler
{
  public:
    using is_ok_fn = std::function<bool(const T &)>;
    using match_fn = std::function<void(const T &)>;
    using transaction_fn =
        std::function<void(const char *src, const char *dst)>;

  public:
    error_handler()
        : _base{[](const char *src, const char *dst) {}}
        , _sm{_base}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }
    explicit error_handler(transaction_fn &&on_transition)
        : _base{std::move(on_transition)}
        , _sm{_base}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }
    explicit error_handler(is_ok_fn &&is_ok)
        : _base{[](const char *src, const char *dst) {}}
        , _sm{_base}
        , _is_ok{std::move(is_ok)}
    {
    }
    explicit error_handler(T ok)
        : _base{[](const char *src, const char *dst) {}}
        , _sm{_base}
        , _is_ok{[ok](const T &t) { return t == ok; }}
    {
    }
    error_handler(is_ok_fn &&is_ok, transaction_fn &&on_transition) noexcept
        : _base{std::move(on_transition)}
        , _sm{_base}
        , _is_ok{std::move(is_ok)}
    {
    }
    ~error_handler() { _sm.process_event(detail::reset{}); }

    error_handler(const error_handler &)                = delete;
    error_handler &operator=(const error_handler &)     = delete;
    error_handler(error_handler &&) noexcept            = default;
    error_handler &operator=(error_handler &&) noexcept = default;

    inline error_handler<T> &match(T &err, match_fn &&cb = nullptr) noexcept
    {
        if(_is_ok(err))
        {
            _sm.process_event(detail::finish{});
        } else
        {
            _sm.process_event(detail::err_event<T>{err, std::move(cb)});
        }

        return *this;
    }

    inline error_handler<T> &finish() noexcept
    {
        _sm.process_event(detail::finish{});
        return *this;
    }

    inline error_handler<T> &fail() noexcept
    {
        _sm.process_event(detail::fail{});
        return *this;
    }

    inline error_handler<T> &abort() noexcept
    {
        _sm.process_event(detail::abort{});
        return *this;
    }

    inline error_handler<T> &reset() noexcept
    {
        _sm.process_event(detail::reset{});
        return *this;
    }

    inline bool is_idle() const noexcept
    {
        return _sm.is(boost::sml::state<detail::idle>);
    }

    inline bool is_handling() const noexcept
    {
        return _sm.is(boost::sml::state<detail::handling>);
    }

    inline bool is_succed() const noexcept
    {
        return _sm.is(boost::sml::state<detail::succed>);
    }

    inline bool is_failed() const noexcept
    {
        return _sm.is(boost::sml::state<detail::failed>);
    }

    inline err_status status() const noexcept
    {
        if(_sm.is(boost::sml::state<detail::idle>))
            return err_status::idle;
        if(_sm.is(boost::sml::state<detail::handling>))
            return err_status::handling;
        if(_sm.is(boost::sml::state<detail::succed>))
            return err_status::succed;
        if(_sm.is(boost::sml::state<detail::failed>))
            return err_status::failed;

        return err_status::unknown;
    }

  private:
    detail::error_handler_impl<T> _base;
    boost::sml::sm<detail::error_handler_impl<T>,
                   boost::sml::defer_queue<std::deque>>
        _sm;

    std::function<bool(const T &)> _is_ok;
};

} // namespace hj

#endif