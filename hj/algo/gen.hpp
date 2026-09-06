/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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