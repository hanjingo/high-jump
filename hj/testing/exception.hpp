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

#include <iostream>
#include <stdexcept>
#include <utility>
#include <typeinfo>

namespace hj
{

static inline void throw_if_false(bool target, const char *memo = "false")
{
    if(!target)
        throw std::logic_error(memo);
}

static inline void throw_if_not_false(bool        target,
                                      const char *memo = "not false")
{
    if(target)
        throw std::logic_error(memo);
}

template <typename T>
static inline void
throw_if_equal(const T &target1, const T &target2, const char *memo = "equal")
{
    if(target1 == target2)
        throw std::logic_error(memo);
}

template <typename T>
static inline void throw_if_not_equal(const T    &target1,
                                      const T    &target2,
                                      const char *memo = "not equal")
{
    if(target1 != target2)
        throw std::logic_error(memo);
}

template <typename T>
static inline void throw_if_empty(const T &target, const char *memo = "empty")
{
    if(target.empty())
        throw std::logic_error(memo);
}

template <typename T>
static inline void throw_if_not_empty(const T    &target,
                                      const char *memo = "not empty")
{
    if(!target.empty())
        throw std::logic_error(memo);
}

template <typename T>
static inline void throw_if_null(T target, const char *memo = "null")
{
    if(target == nullptr || target == NULL)
        throw std::logic_error(memo);
}

template <typename T>
static inline void throw_if_not_null(T target, const char *memo = "not null")
{
    if(target != nullptr && target != NULL)
        throw std::logic_error(memo);
}

template <typename Container, typename T>
static inline void throw_if_exists(const Container &container,
                                   const T         &target,
                                   const char      *memo = "already exist")
{
    for(auto &elem : container)
    {
        if(elem != target)
            continue;

        throw std::logic_error(memo);
    }
}

template <typename Container, typename T>
static inline void throw_if_not_exists(const Container &container,
                                       const T         &target,
                                       const char      *memo = "not exist")
{
    for(auto &elem : container)
    {
        if(elem == target)
            return;
    }

    throw std::logic_error(memo);
}

template <typename Func, typename Handler>
static inline auto recover(Func &&func, Handler &&handler) -> decltype(func())
{
    try
    {
        return func();
    }
    catch(const hj::Exception &e)
    {
        if(detail::is_valid_handler(handler))
        {
            handler(std::current_exception(), e.trace());
        }
    }
    catch(const std::exception &e)
    {
        if(detail::is_valid_handler(handler))
        {
            handler(std::current_exception(), current_stacktrace());
        }
    }
    catch(...)
    {
        if(detail::is_valid_handler(handler))
        {
            handler(std::current_exception(), current_stacktrace());
        }
    }
}

}

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

#endif