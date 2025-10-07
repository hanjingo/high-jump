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

#ifndef ONCE_HPP
#define ONCE_HPP

#include <mutex>

namespace hj
{

#define __once_cat(a, b) a##b
#define _once_cat(a, b) __once_cat(a, b)

#define ONCE(cmd)                                                              \
    static std::once_flag _once_cat(do_once_, __LINE__);                       \
    std::call_once(_once_cat(do_once_, __LINE__), [&]() noexcept {             \
        try                                                                    \
        {                                                                      \
            cmd                                                                \
        }                                                                      \
        catch(...)                                                             \
        {                                                                      \
        }                                                                      \
    });

}

#endif