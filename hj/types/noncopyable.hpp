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

#if __has_include(<boost/config.hpp>)
#include <boost/noncopyable.hpp>

namespace hj
{

using noncopyable = boost::noncopyable;

}

#else
namespace hj
{

class noncopyable
{
  protected:
    noncopyable()                               = default;
    ~noncopyable()                              = default;
    noncopyable(const noncopyable &)            = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

}

#endif


#define DISABLE_COPY(Class)                                                    \
    Class(const Class &)            = delete;                                  \
    Class &operator=(const Class &) = delete;

#define DISABLE_MOVE(Class)                                                    \
    Class(Class &&)            = delete;                                       \
    Class &operator=(Class &&) = delete;

#define DISABLE_COPY_MOVE(Class)                                               \
    DISABLE_COPY(Class)                                                        \
    DISABLE_MOVE(Class)

#endif