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

#ifndef ANY_HPP
#define ANY_HPP

#include <utility>

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <any>

namespace hj
{
using any          = std::any;
using bad_any_cast = std::bad_any_cast;

template <class T>
inline T *any_cast(any *o) noexcept
{
    return std::any_cast<T>(o);
}
template <class T>
inline const T *any_cast(const any *o) noexcept
{
    return std::any_cast<T>(o);
}
template <class T>
inline T any_cast(any &o)
{
    return std::any_cast<T>(o);
}
template <class T>
inline T any_cast(const any &o)
{
    return std::any_cast<T>(o);
}
template <class T>
inline T any_cast(any &&o)
{
    return std::any_cast<T>(std::move(o));
}
}

#else
#include <boost/any.hpp>

namespace hj
{
class any : public boost::any
{
  public:
    using boost::any::any;
    any()
        : boost::any()
    {
    }

    bool has_value() const noexcept { return !this->empty(); }
};

using bad_any_cast = boost::bad_any_cast;

template <class T>
inline T *any_cast(any *o) noexcept
{
    return boost::any_cast<T>(o);
}
template <class T>
inline const T *any_cast(const any *o) noexcept
{
    return boost::any_cast<T>(o);
}
template <class T>
inline T any_cast(any &o)
{
    return boost::any_cast<T>(o);
}
template <class T>
inline T any_cast(const any &o)
{
    return boost::any_cast<T>(o);
}
template <class T>
inline T any_cast(any &&o)
{
    return boost::any_cast<T>(std::move(o));
}
}
#endif

#endif