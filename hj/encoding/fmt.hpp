/*
 *  This file is part of hj.
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

#ifndef FMT_HPP
#define FMT_HPP

#include <fmt/core.h>

namespace hj
{

template <typename... Args>
std::string fmt(const char *style, Args &&...args)
{
    return fmt::vformat(style,
                        fmt::make_format_args(std::forward<Args>(args)...));
}

} // namespace hj

#endif // FMT_HPP