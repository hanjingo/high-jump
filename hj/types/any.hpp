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

#ifndef ANY_HPP
#define ANY_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <any>

namespace hj
{

using any = std::any;

template <class T>
T *any_cast(any *operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
const T *any_cast(const any *operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
T any_cast(any &operand)
{
    return std::any_cast<T>(operand);
}

template <class T>
T any_cast(const any &operand)
{
    return std::any_cast<T>(operand);
}

}

#else
#include <boost/any.hpp>

namespace hj
{

using any = boost::any;

template <class T>
T *any_cast(any *operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
const T *any_cast(const any *operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
T any_cast(any &operand)
{
    return boost::any_cast<T>(operand);
}

template <class T>
T any_cast(const any &operand)
{
    return boost::any_cast<T>(operand);
}

}
#endif

#endif