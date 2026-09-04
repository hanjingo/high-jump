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
#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <boost/stacktrace.hpp>

namespace hj
{

class Exception : public std::runtime_error
{
  public:
    explicit Exception(
        const std::string            &message,
        boost::stacktrace::stacktrace trace = boost::stacktrace::stacktrace())
        : std::runtime_error(message)
        , trace_(std::move(trace))
    {
    }

    explicit Exception(
        const char                   *message,
        boost::stacktrace::stacktrace trace = boost::stacktrace::stacktrace())
        : std::runtime_error(message)
        , trace_(std::move(trace))
    {
    }

    [[nodiscard]] const boost::stacktrace::stacktrace &trace() const noexcept
    {
        return trace_;
    }

  private:
    boost::stacktrace::stacktrace trace_;
};

class NotFoundException : public Exception
{
  public:
    using Exception::Exception;
};

namespace detail
{
template <typename Handler, typename = void>
struct is_valid_handler : std::false_type
{
};

template <typename Handler>
struct is_valid_handler<Handler,
                        std::void_t<decltype(std::declval<Handler>()(
                            std::declval<std::exception_ptr>(),
                            std::declval<boost::stacktrace::stacktrace>()))>>
    : std::true_type
{
};

template <typename Handler>
inline constexpr bool is_valid_handler_v = is_valid_handler<Handler>::value;

template <typename C, typename T, typename = void>
struct has_find : std::false_type
{
};

template <typename C, typename T>
struct has_find<C,
                T,
                std::void_t<decltype(static_cast<bool>(
                    std::declval<const C &>().find(std::declval<const T &>())
                    != std::declval<const C &>().end()))>> : std::true_type
{
};

template <typename C, typename T>
inline constexpr bool has_find_v = has_find<C, T>::value;

template <typename T>
struct recover_return_type
{
    using type = std::optional<T>;
};

template <>
struct recover_return_type<void>
{
    using type = void;
};

template <typename T>
using recover_return_type_t = typename recover_return_type<T>::type;

template <typename T>
struct is_optional : std::false_type
{
};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_optional_v = is_optional<std::decay_t<T>>::value;

template <typename T, typename = void>
struct is_nullable : std::false_type
{
};

template <typename T>
struct is_nullable<
    T,
    std::enable_if_t<std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>>>
    : std::true_type
{
};

template <typename T>
struct is_nullable<
    T,
    std::enable_if_t<
        !std::is_pointer_v<T> && !std::is_same_v<T, std::nullptr_t>,
        std::void_t<decltype(static_cast<bool>(std::declval<const T &>()))>>>
    : std::true_type
{
};

template <typename T>
inline constexpr bool is_nullable_v = is_nullable<std::decay_t<T>>::value;

} // namespace detail

[[nodiscard]]
inline boost::stacktrace::stacktrace current_stacktrace()
{
    return boost::stacktrace::stacktrace();
}

inline void throw_if_false(bool target, std::string_view memo = "false")
{
    if(!target)
        throw std::logic_error(std::string(memo));
}

inline void throw_if_not_false(bool target, std::string_view memo = "not false")
{
    if(target)
        throw std::logic_error(std::string(memo));
}

template <typename T>
inline void throw_if_equal(const T         &target1,
                           const T         &target2,
                           std::string_view memo = "equal")
{
    if(target1 == target2)
        throw std::logic_error(std::string(memo));
}

template <typename T>
inline void throw_if_not_equal(const T         &target1,
                               const T         &target2,
                               std::string_view memo = "not equal")
{
    if(target1 != target2)
        throw std::logic_error(std::string(memo));
}

template <typename T>
inline void throw_if_empty(const T &target, std::string_view memo = "empty")
{
    if(target.empty())
        throw std::invalid_argument(std::string(memo));
}

template <typename T>
inline void throw_if_not_empty(const T         &target,
                               std::string_view memo = "not empty")
{
    if(!target.empty())
        throw std::invalid_argument(std::string(memo));
}

template <typename T>
inline void throw_if_null(const T &target, std::string_view memo = "null")
{
    static_assert(detail::is_nullable_v<T>,
                  "hj::throw_if_null requires a nullable pointer, smart "
                  "pointer, or optional type.");

    bool is_null = false;
    if constexpr(detail::is_optional_v<T>)
    {
        is_null = !target.has_value();
    } else
    {
        is_null = (target == nullptr);
    }

    if(is_null)
        throw std::invalid_argument(std::string(memo));
}

template <typename T>
inline void throw_if_not_null(const T         &target,
                              std::string_view memo = "not null")
{
    static_assert(detail::is_nullable_v<T>,
                  "hj::throw_if_not_null requires a nullable pointer, smart "
                  "pointer, or optional type.");

    bool is_null = false;
    if constexpr(detail::is_optional_v<T>)
    {
        is_null = !target.has_value();
    } else
    {
        is_null = (target == nullptr);
    }

    if(!is_null)
        throw std::invalid_argument(std::string(memo));
}

template <typename Container, typename T>
inline void throw_if_exists(const Container &container,
                            const T         &target,
                            std::string_view memo = "already exist")
{
    bool exists = false;
    if constexpr(detail::has_find_v<Container, T>)
    {
        exists = (container.find(target) != container.end());
    } else
    {
        exists = (std::find(std::begin(container), std::end(container), target)
                  != std::end(container));
    }

    if(exists)
        throw std::invalid_argument(std::string(memo));
}

template <typename Container, typename T>
inline void throw_if_not_exists(const Container &container,
                                const T         &target,
                                std::string_view memo = "not exist")
{
    bool exists = false;
    if constexpr(detail::has_find_v<Container, T>)
    {
        exists = (container.find(target) != container.end());
    } else
    {
        exists = (std::find(std::begin(container), std::end(container), target)
                  != std::end(container));
    }

    if(!exists)
        throw std::invalid_argument(std::string(memo));
}

template <typename Func, typename Handler>
inline auto recover(Func &&func, Handler &&handler)
    -> detail::recover_return_type_t<std::invoke_result_t<Func>>
{
    using RawReturnType = std::invoke_result_t<Func>;

    static_assert(
        !std::is_reference_v<RawReturnType>,
        "hj::recover does not support functions that return references. "
        "Please return by value.");

    try
    {
        if constexpr(std::is_void_v<RawReturnType>)
        {
            func();
            return;
        } else
        {
            return func();
        }
    }
    catch(const hj::Exception &e)
    {
        if constexpr(detail::is_valid_handler_v<Handler>)
        {
            handler(std::current_exception(), e.trace());
        }
    }
    catch(const std::exception &)
    {
        if constexpr(detail::is_valid_handler_v<Handler>)
        {
            handler(std::current_exception(), current_stacktrace());
        }
    }
    catch(...)
    {
        if constexpr(detail::is_valid_handler_v<Handler>)
        {
            handler(std::current_exception(), current_stacktrace());
        }
    }

    if constexpr(!std::is_void_v<RawReturnType>)
    {
        return std::nullopt;
    }
}

} // namespace hj

#define HJ_RECOVER_WITH_LOG(cmd, os)                                           \
    do                                                                         \
    {                                                                          \
        ::hj::recover(                                                         \
            [&]() { cmd; },                                                    \
            [&](const std::exception_ptr            &ep,                       \
                const boost::stacktrace::stacktrace &st) {                     \
                try                                                            \
                {                                                              \
                    if(ep)                                                     \
                        std::rethrow_exception(ep);                            \
                }                                                              \
                catch(const std::exception &e)                                 \
                {                                                              \
                    (os) << "Exception caught: " << e.what() << "\n"           \
                         << "Stacktrace:\n"                                    \
                         << st << std::endl;                                   \
                }                                                              \
                catch(...)                                                     \
                {                                                              \
                    (os) << "Unknown exception caught.\nStacktrace:\n"         \
                         << st << std::endl;                                   \
                }                                                              \
            });                                                                \
    } while(0)

#define HJ_RECOVER(cmd) HJ_RECOVER_WITH_LOG(cmd, std::cerr)

#endif // EXCEPTION_HPP