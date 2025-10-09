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
#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <boost/coroutine2/all.hpp>
#include <exception>
#include <iostream>

namespace hj
{

// See Also: https://www.boost.org/doc/libs/1_75_0/libs/coroutine2/doc/html/index.html

template <typename T>
using coroutine = boost::coroutines2::coroutine<T>;

using stack_alloc = boost::context::fixedsize_stack;

}

#define __coroutine_cat(a, b) a##b
#define _coroutine_cat(a, b) __coroutine_cat(a, b)

#define COROUTINE(cmd)                                                         \
    hj::coroutine<void>::pull_type __coroutine_cat(__coroutine__,              \
                                                   __COUNTER__)(               \
        [&](hj::coroutine<void>::push_type &) {                                \
            try                                                                \
            {                                                                  \
                cmd;                                                           \
            }                                                                  \
            catch(const std::exception &e)                                     \
            {                                                                  \
                std::cerr << "Coroutine exception: " << e.what() << std::endl; \
            }                                                                  \
        });

#endif