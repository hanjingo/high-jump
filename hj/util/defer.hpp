/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  Licensed under the GNU General Public License, Version 3.0.
 */

#ifndef DEFER_HPP
#define DEFER_HPP

#include <utility>
#include <type_traits>

namespace hj
{

struct empty_defer_t
{
    void operator()() const noexcept {}
};

template <typename F = empty_defer_t>
class [[nodiscard]] defer final
{
  public:
    defer() noexcept
        : _active(false)
    {
    }

    template <
        typename Fn,
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, defer>>>
    explicit defer(Fn &&f) noexcept(std::is_nothrow_constructible_v<F, Fn>)
        : _cb(std::forward<Fn>(f))
        , _active(true)
    {
    }

    ~defer() noexcept
    {
        if(_active)
        {
            try
            {
                _cb();
            }
            catch(...)
            {
            }
        }
    }

    defer(const defer &)            = delete;
    defer &operator=(const defer &) = delete;
    defer(defer &&other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : _cb(std::move(other._cb))
        , _active(other._active)
    {
        other._active = false;
    }

    defer &operator=(defer &&) = delete;

  private:
    F    _cb;
    bool _active = false;
};

template <typename Fn>
defer(Fn &&) -> defer<std::decay_t<Fn>>;

} // namespace hj

#define HJ_DEFER_CAT_IMPL(a, b) a##b
#define HJ_DEFER_CAT(a, b) HJ_DEFER_CAT_IMPL(a, b)

#define HJ_DEFER(cmd)                                                          \
    auto HJ_DEFER_CAT(_hj_defer_obj_, __COUNTER__) = ::hj::defer([&]() {       \
        try                                                                    \
        {                                                                      \
            cmd;                                                               \
        }                                                                      \
        catch(...)                                                             \
        {                                                                      \
        }                                                                      \
    })

#define HJ_DEFER_CLASS(cmd)                                                    \
    auto HJ_DEFER_CAT(_hj_defer_class_obj_, __COUNTER__) =                     \
        ::hj::defer([&, this]() {                                              \
            try                                                                \
            {                                                                  \
                cmd;                                                           \
            }                                                                  \
            catch(...)                                                         \
            {                                                                  \
            }                                                                  \
        })

#endif // DEFER_HPP