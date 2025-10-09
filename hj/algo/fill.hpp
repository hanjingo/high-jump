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

#ifndef FILL_HPP
#define FILL_HPP

#include <functional>

namespace hj
{

template <typename Container, typename Fn>
inline void fill(Container &ct, Fn &&fn)
{
    size_t idx = 0;
    for(auto &v : ct)
        v = std::forward<Fn>(fn)(idx++);
}

template <typename Container, typename Fn>
inline void fill(Container &ct, Fn &&fn, const size_t n)
{
    size_t idx = 0;
    for(auto &v : ct)
    {
        v = std::forward<Fn>(fn)(idx++);
        if(idx >= n)
            return;
    }
}

template <typename Iterator, typename Fn>
inline void fill(Iterator begin, Iterator end, Fn &&fn)
{
    size_t idx = 0;
    for(auto itr = begin; itr != end; ++itr)
        *itr = std::forward<Fn>(fn)(idx++);
}

}

#endif