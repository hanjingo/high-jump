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

#ifndef HJ_GEN_HPP
#define HJ_GEN_HPP

#include <cstddef>
#include <utility>
#include <iterator>
#include <type_traits>

namespace hj
{

template <typename ForwardIterator, typename Fn>
constexpr ForwardIterator
gen(ForwardIterator begin, ForwardIterator end, Fn &&fn) noexcept(
    noexcept(*begin = fn(std::declval<std::size_t &>())) && noexcept(++begin))
{
    std::size_t idx = 0;
    for(auto itr = begin; itr != end; ++itr)
        *itr = fn(idx++);

    return begin;
}

template <
    typename Container,
    typename Fn,
    typename = std::void_t<decltype(std::begin(std::declval<Container &>())),
                           decltype(std::end(std::declval<Container &>()))>>
constexpr decltype(auto) gen(Container &ct, Fn &&fn)
{
    using std::begin;
    using std::end;
    return ::hj::gen(begin(ct), end(ct), std::forward<Fn>(fn));
}

template <
    typename OutputIterator,
    typename Size,
    typename Fn,
    typename = typename std::iterator_traits<OutputIterator>::iterator_category>
constexpr OutputIterator gen_n(OutputIterator first, Size n, Fn &&fn)
{
    std::size_t idx = 0;
    for(Size i = 0; i < n; ++i)
    {
        *first = fn(idx++);
        ++first;
    }
    return first;
}

template <
    typename Container,
    typename Fn,
    typename = std::void_t<decltype(std::begin(std::declval<Container &>())),
                           decltype(std::end(std::declval<Container &>()))>>
constexpr std::size_t gen_n(Container &ct, std::size_t n, Fn &&fn)
{
    using std::begin;
    using std::end;
    auto itr  = begin(ct);
    auto last = end(ct);

    std::size_t idx = 0;
    while(itr != last && idx < n)
    {
        *itr = fn(idx++);
        ++itr;
    }

    return idx;
}

} // namespace hj

#endif // HJ_GEN_HPP