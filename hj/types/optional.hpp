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

#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <optional>
#define HJ_USE_STD_OPTIONAL
#else
#include <boost/optional.hpp>
#include <boost/optional/bad_optional_access.hpp>
#define HJ_USE_BOOST_OPTIONAL
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define HJ_NODISCARD [[nodiscard]]
#else
#define HJ_NODISCARD
#endif
#else
#define HJ_NODISCARD
#endif

namespace hj
{
#ifdef HJ_USE_STD_OPTIONAL
using nullopt_t                    = std::nullopt_t;
inline constexpr nullopt_t nullopt = std::nullopt;
#else
using nullopt_t                = boost::none_t;
inline const nullopt_t nullopt = boost::none;
#endif

template <class T>
class optional
{
  private:
#ifdef HJ_USE_STD_OPTIONAL
    std::optional<T> _impl;
#else
    boost::optional<T> _impl;
#endif

  public:
    constexpr optional() noexcept = default;

    constexpr optional(nullopt_t) noexcept
        : _impl(
#ifdef HJ_USE_STD_OPTIONAL
              std::nullopt
#else
              boost::none
#endif
          )
    {
    }

    template <
        class U               = T,
        std::enable_if_t<!std::is_same_v<std::decay_t<U>, optional>
                             && !std::is_same_v<std::decay_t<U>, nullopt_t>,
                         int> = 0>
    constexpr optional(U &&value)
        : _impl(std::forward<U>(value))
    {
    }

    optional(const optional &) = default;
    constexpr optional(optional &&other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : _impl(std::move(other._impl))
    {
        other.reset();
    }

    optional &operator=(const optional &) = default;
    optional &
    operator=(optional &&other) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        if(this != &other)
        {
            _impl = std::move(other._impl);
            other.reset();
        }
        return *this;
    }

    optional &operator=(nullopt_t) noexcept
    {
        _impl =
#ifdef HJ_USE_STD_OPTIONAL
            std::nullopt;
#else
            boost::none;
#endif
        return *this;
    }

    HJ_NODISCARD bool has_value() const noexcept
    {
#ifdef HJ_USE_STD_OPTIONAL
        return _impl.has_value();
#else
        return _impl.is_initialized();
#endif
    }

    constexpr explicit operator bool() const noexcept { return has_value(); }

    HJ_NODISCARD auto value() & { return _impl.value(); }
    HJ_NODISCARD auto value() const & { return _impl.value(); }
    HJ_NODISCARD auto value() && { return std::move(_impl).value(); }

    template <class U>
    HJ_NODISCARD T value_or(U &&default_value) const &
    {
        return has_value() ? **this
                           : static_cast<T>(std::forward<U>(default_value));
    }

    template <class U>
    HJ_NODISCARD T value_or(U &&default_value) &&
    {
        return has_value() ? std::move(**this)
                           : static_cast<T>(std::forward<U>(default_value));
    }

    HJ_NODISCARD const T *operator->() const noexcept { return &*_impl; }
    HJ_NODISCARD T       *operator->() noexcept { return &*_impl; }

    HJ_NODISCARD const T &operator*() const & noexcept { return *_impl; }
    HJ_NODISCARD T       &operator*()       &noexcept { return *_impl; }

    template <class... Args>
    void emplace(Args &&...args)
    {
        _impl.emplace(std::forward<Args>(args)...);
    }

    void reset() noexcept
    {
#ifdef HJ_USE_STD_OPTIONAL
        _impl.reset();
#else
        _impl = boost::none;
#endif
    }

    void swap(optional &other) noexcept(noexcept(_impl.swap(other._impl)))
    {
        _impl.swap(other._impl);
    }
};

} // namespace hj

#undef HJ_NODISCARD

#endif // OPTIONAL_HPP