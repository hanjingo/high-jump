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

#ifndef VARIANT_HPP
#define VARIANT_HPP

#if __has_include(<variant>)
#include <variant>

namespace hj
{
template <typename... T>
using variant = std::variant<T...>;

template <class T, class... Types>
constexpr T &get(std::variant<Types...> &v)
{
    return std::get<T>(v);
};

template <class T, class... Types>
constexpr T &&get(std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
};

template <class T, class... Types>
constexpr const T &get(const std::variant<Types...> &v)
{
    return std::get<T>(v);
};

template <class T, class... Types>
constexpr const T &&get(const std::variant<Types...> &&v)
{
    return std::get<T>(std::move(v));
};

}

#elif __has_include(<boost/variant.hpp>)
#include <boost/variant.hpp>

namespace hj
{
template <typename... T>
using variant = boost::variant<T...>;

template <class T, class... Types>
constexpr T &get(boost::variant<Types...> &v)
{
    return boost::get<T>(v);
};

template <class T, class... Types>
constexpr T &&get(boost::variant<Types...> &&v)
{
    return boost::get<T>(boost::move(v));
};

template <class T, class... Types>
constexpr const T &get(const boost::variant<Types...> &v)
{
    return boost::get<T>(v);
};

template <class T, class... Types>
constexpr const T &&get(const boost::variant<Types...> &&v)
{
    return boost::get<T>(std::move(v));
};

}

#else
#error                                                                         \
    "No suitable variant implementation found (need C++17 std::variant or boost::variant)"

#endif

#endif