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

#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#if __has_include(<string_view>)
#include <string_view>
namespace hj
{
using string_view = std::string_view;
}

#elif __has_include(<boost/utility/string_view.hpp>)
#include <boost/utility/string_view.hpp>
namespace hj
{
using string_view = boost::string_view;
}

#else
#error                                                                         \
    "No suitable string_view implementation found (need C++17 std::string_view or boost::string_view)"

#endif

#endif // STRING_VIEW_HPP