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

#ifndef REFLECT_HPP
#define REFLECT_HPP

#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <cstring>

#include <boost/core/demangle.hpp>

namespace hj
{
class reflect
{
  public:
    template <typename T>
    static std::string type_name(const T &t)
    {
#if defined(__GNUC__) || defined(__clang__)
        return boost::core::demangle(typeid(t).name());
#else
        return typeid(t).name();
#endif
    }

    template <typename T>
    static bool is_pod(T t)
    {
        return std::is_pod<T>::value;
    }

    template <typename T>
    static T copy(const T &t)
    {
        return t;
    }

    template <typename T>
    static T clone(const T &t)
    {
        return T(t);
    }

    template <typename T>
    static std::vector<unsigned char> serialize(const T &t)
    {
        static_assert(std::is_trivially_copyable<T>::value, "POD only");
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(&t);
        return std::vector<unsigned char>(ptr, ptr + sizeof(T));
    }

    template <typename T>
    static T unserialize(const unsigned char *buf)
    {
        static_assert(std::is_trivially_copyable<T>::value, "POD only");
        T t;
        std::memcpy(&t, buf, sizeof(T));
        return t;
    }

    template <typename T, typename Member>
    static std::size_t offset_of(Member T::*member)
    {
        return reinterpret_cast<std::size_t>(&(((T *) 0)->*member));
    }

    template <typename T, typename Member>
    static std::size_t size_of(Member T::*member)
    {
        return sizeof(((T *) 0)->*member);
    }

    template <typename T, typename Member>
    static std::size_t align_of(Member T::*member)
    {
        return alignof(Member);
    }

    template <typename T, typename... Others>
    static bool is_same_type(const T &, const Others &...)
    {
        return (std::conjunction<std::is_same<T, Others>...>::value);
    }

  private:
    reflect()                           = default;
    ~reflect()                          = default;
    reflect(const reflect &)            = delete;
    reflect &operator=(const reflect &) = delete;
    reflect(reflect &&)                 = delete;
    reflect &operator=(reflect &&)      = delete;
};

}

#endif
