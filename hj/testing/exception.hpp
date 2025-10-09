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
#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <stdexcept>

namespace hj
{

static void throw_if_false(bool target, const char *memo = "false")
{
    if(!target)
        throw std::logic_error(memo);
}

static void throw_if_not_false(bool target, const char *memo = "not false")
{
    if(target)
        throw std::logic_error(memo);
}

template <typename T>
static void
throw_if_equal(const T &target1, const T &target2, const char *memo = "equal")
{
    if(target1 == target2)
        throw std::logic_error(memo);
}

template <typename T>
static void throw_if_not_equal(const T    &target1,
                               const T    &target2,
                               const char *memo = "not equal")
{
    if(target1 != target2)
        throw std::logic_error(memo);
}

template <typename T>
static void throw_if_empty(const T &target, const char *memo = "empty")
{
    if(target.empty())
        throw std::logic_error(memo);
}

template <typename T>
static void throw_if_not_empty(const T &target, const char *memo = "not empty")
{
    if(!target.empty())
        throw std::logic_error(memo);
}

template <typename T>
static void throw_if_null(T target, const char *memo = "null")
{
    if(target == nullptr || target == NULL)
        throw std::logic_error(memo);
}

template <typename T>
static void throw_if_not_null(T target, const char *memo = "not null")
{
    if(target != nullptr && target != NULL)
        throw std::logic_error(memo);
}

template <typename Container, typename T>
static void throw_if_exists(const Container &container,
                            const T         &target,
                            const char      *memo = "already exist")
{
    for(auto &elem : container)
    {
        if(elem != target)
            continue;

        throw std::logic_error(memo);
    }
}

template <typename Container, typename T>
static void throw_if_not_exists(const Container &container,
                                const T         &target,
                                const char      *memo = "not exist")
{
    for(auto &elem : container)
    {
        if(elem == target)
            return;
    }

    throw std::logic_error(memo);
}

}

#endif