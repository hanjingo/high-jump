/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#if defined(__has_include)
#if __has_include(<boost/noncopyable.hpp>)
#include <boost/noncopyable.hpp>
#define HJ_HAS_BOOST_NONCOPYABLE 1
#endif
#endif

namespace hj
{
#if defined(HJ_HAS_BOOST_NONCOPYABLE)
using noncopyable = boost::noncopyable;
#else
class noncopyable
{
  protected:
    noncopyable()  = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable &)            = delete;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable(noncopyable &&)                 = delete;
    noncopyable &operator=(noncopyable &&)      = delete;
};
#endif
}

#define HJ_DISABLE_COPY(Class)                                                 \
    Class(const Class &)            = delete;                                  \
    Class &operator=(const Class &) = delete;

#define HJ_DISABLE_MOVE(Class)                                                 \
    Class(Class &&)            = delete;                                       \
    Class &operator=(Class &&) = delete;

#define HJ_DISABLE_COPY_MOVE(Class)                                            \
    HJ_DISABLE_COPY(Class)                                                     \
    HJ_DISABLE_MOVE(Class)

#endif