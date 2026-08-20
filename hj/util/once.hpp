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

#define HJ_ONCE_CAT_IMPL(a, b) a##b
#define HJ_ONCE_CAT(a, b) HJ_ONCE_CAT_IMPL(a, b)

#define HJ_ONCE_IMPL(counter_val, ...)                                         \
    do                                                                         \
    {                                                                          \
        static std::once_flag HJ_ONCE_CAT(do_once_flag_, counter_val);         \
        std::call_once(HJ_ONCE_CAT(do_once_flag_, counter_val),                \
                       [&]() { __VA_ARGS__ });                                 \
    } while(0)

#define HJ_ONCE(...) HJ_ONCE_IMPL(__COUNTER__, __VA_ARGS__)

#endif