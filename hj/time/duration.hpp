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
#ifndef DURATION_HPP
#define DURATION_HPP

#include <chrono>

namespace hj
{
static std::chrono::time_point<std::chrono::high_resolution_clock> tm_start =
    std::chrono::high_resolution_clock::now();
}

#define TIME_START() hj::tm_start = std::chrono::high_resolution_clock::now();

#define TIME_PASSED() (std::chrono::high_resolution_clock::now() - hj::tm_start)

#define TIME_PASSED_NS()                                                       \
    std::chrono::duration_cast<std::chrono::nanoseconds>(                      \
        std::chrono::high_resolution_clock::now() - hj::tm_start)              \
        .count()

#define TIME_PASSED_MS()                                                       \
    std::chrono::duration_cast<std::chrono::milliseconds>(                     \
        std::chrono::high_resolution_clock::now() - hj::tm_start)              \
        .count()

#define TIME_PASSED_SEC()                                                      \
    std::chrono::duration_cast<std::chrono::seconds>(                          \
        std::chrono::high_resolution_clock::now() - hj::tm_start)              \
        .count()

#endif