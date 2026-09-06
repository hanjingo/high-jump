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

#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <cstdint>
#include <cstring>
#include <type_traits>

#if defined(_MSC_VER)
#include <stdlib.h>
#endif

namespace hj
{

namespace detail
{

inline constexpr bool is_big_endian_impl() noexcept
{
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)                   \
    && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return true;
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)              \
    && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return false;
#elif defined(_MSC_VER)
    return false;
#else
    uint16_t val = 0x0100;
    uint8_t  bytes[sizeof(uint16_t)];
    std::memcpy(bytes, &val, sizeof(uint16_t));
    return bytes[0] == 0x01;
#endif
}

inline uint16_t bswap16(uint16_t x) noexcept
{
#if defined(_MSC_VER)
    return _byteswap_ushort(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap16(x);
#else
    return static_cast<uint16_t>((x >> 8) | (x << 8));
#endif
}

inline uint32_t bswap32(uint32_t x) noexcept
{
#if defined(_MSC_VER)
    return _byteswap_ulong(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#else
    return ((x >> 24) & 0x000000FFu) | ((x >> 8) & 0x0000FF00u)
           | ((x << 8) & 0x00FF0000u) | ((x << 24) & 0xFF000000u);
#endif
}

inline uint64_t bswap64(uint64_t x) noexcept
{
#if defined(_MSC_VER)
    return _byteswap_uint64(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(x);
#else
    return ((x & 0xFF00000000000000ull) >> 56)
           | ((x & 0x00FF000000000000ull) >> 40)
           | ((x & 0x0000FF0000000000ull) >> 24)
           | ((x & 0x000000FF00000000ull) >> 8)
           | ((x & 0x00000000FF000000ull) << 8)
           | ((x & 0x0000000000FF0000ull) << 24)
           | ((x & 0x000000000000FF00ull) << 40)
           | ((x & 0x00000000000000FFull) << 56);
#endif
}

template <typename T>
inline T byte_swap(T val) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT u_val = 0;
    std::memcpy(&u_val, &val, sizeof(T));

    if constexpr(sizeof(T) == 2)
    {
        u_val = static_cast<UnsignedT>(bswap16(static_cast<uint16_t>(u_val)));
    } else if constexpr(sizeof(T) == 4)
    {
        u_val = static_cast<UnsignedT>(bswap32(static_cast<uint32_t>(u_val)));
    } else if constexpr(sizeof(T) == 8)
    {
        u_val = static_cast<UnsignedT>(bswap64(static_cast<uint64_t>(u_val)));
    }

    T res;
    std::memcpy(&res, &u_val, sizeof(T));
    return res;
}

} // namespace detail

inline constexpr bool is_big_endian() noexcept
{
    return detail::is_big_endian_impl();
}

template <typename T>
inline T to_big_endian(T v) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be integral");

    if constexpr(!detail::is_big_endian_impl())
    {
        return detail::byte_swap(v);
    }
    return v;
}

template <typename T>
inline T to_little_endian(T v) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be integral");

    if constexpr(detail::is_big_endian_impl())
    {
        return detail::byte_swap(v);
    }
    return v;
}

} // namespace hj

#endif // ENDIAN_HPP