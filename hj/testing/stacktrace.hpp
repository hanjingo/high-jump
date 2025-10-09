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
#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

#include <boost/stacktrace.hpp>
#include <iostream>

namespace hj
{

inline auto stacktrace()
{
    return boost::stacktrace::stacktrace();
}

} // namespace hj

#define RECOVER(cmd)                                                           \
    try                                                                        \
    {                                                                          \
        cmd                                                                    \
    }                                                                          \
    catch(...)                                                                 \
    {                                                                          \
        std::cerr << hj::stacktrace() << std::endl;                            \
    }

#endif