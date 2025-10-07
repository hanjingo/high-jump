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
#include <type_traits>

#if defined(_WIN32)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#if defined(_MSC_VER)
#include <stdlib.h>
#define ENDIAN_SWAP16 _byteswap_ushort
#define ENDIAN_SWAP32 _byteswap_ulong
#define ENDIAN_SWAP64 _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
#define ENDIAN_SWAP16 __builtin_bswap16
#define ENDIAN_SWAP32 __builtin_bswap32
#define ENDIAN_SWAP64 __builtin_bswap64
#else
inline uint16_t ENDIAN_SWAP16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}
inline uint32_t ENDIAN_SWAP32(uint32_t x)
{
    return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000)
           | ((x << 24) & 0xFF000000);
}
inline uint64_t ENDIAN_SWAP64(uint64_t x)
{
    return ((x & 0xFF00000000000000ull) >> 56)
           | ((x & 0x00FF000000000000ull) >> 40)
           | ((x & 0x0000FF0000000000ull) >> 24)
           | ((x & 0x000000FF00000000ull) >> 8)
           | ((x & 0x00000000FF000000ull) << 8)
           | ((x & 0x0000000000FF0000ull) << 24)
           | ((x & 0x000000000000FF00ull) << 40)
           | ((x & 0x00000000000000FFull) << 56);
}
#endif

namespace hj
{

bool is_big_endian() noexcept
{
    union
    {
        uint32_t i;
        uint8_t  c[4];
    } u = {0x01020304};
    return u.c[0] == 0x01;
}

template <typename T>
constexpr T to_big_endian(T v)
{
    static_assert(std::is_integral<T>::value, "T must be integral");

    if constexpr(sizeof(T) == 2)
        return htons(v);
    else if constexpr(sizeof(T) == 4)
        return htonl(v);
    else if constexpr(sizeof(T) == 8)
        return ((uint64_t) htonl(uint32_t(v >> 32))
                | ((uint64_t) htonl(uint32_t(v & 0xFFFFFFFF)) << 32));
    else
        return v;
}

template <typename T>
constexpr T to_little_endian(T v)
{
    static_assert(std::is_integral<T>::value, "T must be integral");

    if(is_big_endian())
    {
        if constexpr(sizeof(T) == 2)
            return ENDIAN_SWAP16(v);
        else if constexpr(sizeof(T) == 4)
            return ENDIAN_SWAP32(v);
        else if constexpr(sizeof(T) == 8)
            return ENDIAN_SWAP64(v);
    }
    return v;
}

} // namespace hj

#endif // ENDIAN_HPP