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