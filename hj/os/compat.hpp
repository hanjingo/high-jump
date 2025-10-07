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

#ifndef COMPAT_HPP
#define COMPAT_HPP

#include <stdio.h>

// -------------------------------- for c++ --------------------------------
#ifdef __cplusplus
#include <functional>

// c++ std::unary_function compatibility
#ifndef HJ_UNARY_FUNCTION_DEFINED
#if defined(_MSC_VER)
#if (_MSC_VER >= 1910)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif
#elif (__cplusplus >= 201703L)
#if defined(__GLIBCXX__)
#define HJ_UNARY_FUNCTION_DEFINED 1
#elif defined(_HJ_VERSION)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#define HJ_UNARY_FUNCTION_DEFINED 0
#endif
#else
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif
#endif

#if !HJ_UNARY_FUNCTION_DEFINED
namespace std
{
template <class Arg, class Result>
struct unary_function
{
    typedef Arg    argument_type;
    typedef Result result_type;
};
}
#undef HJ_UNARY_FUNCTION_DEFINED
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif

#endif


// ------------------------------------ for c/c++ --------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
// deprecated
#if (__cplusplus >= 201402L)
#if defined(_MSC_VER)
#define DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__)
#define DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define DEPRECATED(msg) [[deprecated(msg)]]
#endif
#else
#define DEPRECATED(msg)
#endif


// inline
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE __attribute__((always_inline)) inline
#define NO_INLINE __attribute__((noinline))
#else
#define FORCE_INLINE inline
#define NO_INLINE
#endif


// branch prediction hints (gcc and clang only)
#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define HOT
#define COLD
#endif

#ifdef __cplusplus
}
#endif

#endif // COMPAT_HPP