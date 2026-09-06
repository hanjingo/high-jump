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

#ifndef BITS_HPP
#define BITS_HPP

#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#if __has_include(<bit>)
#include <bit>
#define HJ_BITS_HAS_STD_BIT 1
#endif
#endif

#if !defined(HJ_BITS_HAS_STD_BIT) && defined(_MSC_VER)
#include <intrin.h>
#endif

namespace hj::bits
{

namespace detail
{
template <typename T>
inline constexpr bool is_valid_bit_type_v =
    std::is_unsigned_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template <typename T>
using enable_if_valid_bit_type_t =
    std::enable_if_t<is_valid_bit_type_v<T>, int>;
} // namespace detail

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool try_get(const T src, const std::size_t pos, bool &val) noexcept
{
    constexpr std::size_t total_bits = sizeof(T) * CHAR_BIT;

    if(pos >= total_bits)
        return false;

    val = static_cast<bool>((src >> pos) & T(1));
    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
[[nodiscard]] constexpr bool get(const T src, const std::size_t pos)
{
    bool val = false;
    if(!try_get(src, pos, val))
        throw std::out_of_range("bit position out of bounds");

    return val;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool
try_put(T &src, const std::size_t pos, const bool bit = true) noexcept
{
    constexpr std::size_t total_bits = sizeof(T) * CHAR_BIT;

    if(pos >= total_bits)
        return false;

    const T mask = T(1) << pos;
    if(bit)
        src |= mask;
    else
        src &= ~mask;

    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &put(T &src, const std::size_t pos, const bool bit = true)
{
    if(!try_put(src, pos, bit))
        throw std::out_of_range("bit position out of bounds");

    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool try_flip(T &src, const std::size_t pos) noexcept
{
    constexpr std::size_t total_bits = sizeof(T) * CHAR_BIT;
    if(pos >= total_bits)
        return false;

    const T mask = T(1) << pos;
    src ^= mask;
    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &flip(T &src, const std::size_t pos)
{
    if(!try_flip(src, pos))
        throw std::out_of_range("bit position out of bounds");

    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &flip(T &src) noexcept
{
    src = ~src;
    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool try_extract(const T           src,
                           const std::size_t offset,
                           const std::size_t width,
                           T                &val) noexcept
{
    constexpr std::size_t total_bits = sizeof(T) * CHAR_BIT;
    if(width == 0 || offset >= total_bits || width > total_bits - offset)
        return false;

    const T mask = (width == total_bits) ? ~T(0) : ((T(1) << width) - T(1));
    val          = (src >> offset) & mask;
    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
[[nodiscard]] constexpr T
extract(const T src, const std::size_t offset, const std::size_t width)
{
    T val = T(0);
    if(!try_extract(src, offset, width, val))
        throw std::out_of_range("bitfield offset or width out of bounds");

    return val;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool try_insert(T                &src,
                          const std::size_t offset,
                          const std::size_t width,
                          const T           value) noexcept
{
    constexpr std::size_t total_bits = sizeof(T) * CHAR_BIT;
    if(width == 0 || offset >= total_bits || width > total_bits - offset)
        return false;

    const T mask = (width == total_bits) ? ~T(0) : ((T(1) << width) - T(1));
    src          = (src & ~(mask << offset)) | ((value & mask) << offset);
    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &
insert(T &src, const std::size_t offset, const std::size_t width, const T value)
{
    if(!try_insert(src, offset, width, value))
        throw std::out_of_range("bitfield offset or width out of bounds");

    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &clear(T &src) noexcept
{
    src = T(0);
    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr T &set_all(T &src) noexcept
{
    src = ~T(0);
    return src;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr bool
to_string(const T &src, char *buf, const std::size_t buf_size) noexcept
{
    constexpr std::size_t sz = sizeof(T) * CHAR_BIT;
    if(buf == nullptr || buf_size <= sz)
        return false;

    for(std::size_t i = 0; i < sz; ++i)
    {
        std::size_t shift = sz - 1 - i;
        buf[i]            = ((src >> shift) & T(1)) ? '1' : '0';
    }

    buf[sz] = '\0';
    return true;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
std::string to_string(const T &src)
{
    constexpr std::size_t sz = sizeof(T) * CHAR_BIT;
    std::string           res(sz, '0');
    to_string(src, res.data(), sz + 1);
    return res;
}

template <typename T, detail::enable_if_valid_bit_type_t<T> = 0>
constexpr int countl_zero(const T &src) noexcept
{
#if defined(HJ_BITS_HAS_STD_BIT)
    return std::countl_zero(src);
#else
    constexpr int total_bits = static_cast<int>(sizeof(T) * CHAR_BIT);

    if(src == 0)
        return total_bits;

#if defined(__GNUC__) || defined(__clang__)
    if constexpr(sizeof(T) == 8)
    {
        return __builtin_clzll(static_cast<uint64_t>(src));
    } else if constexpr(sizeof(T) == 4)
    {
        return __builtin_clz(static_cast<uint32_t>(src));
    } else if constexpr(sizeof(T) == 2)
    {
        return __builtin_clz(static_cast<uint32_t>(src)) - 16;
    } else if constexpr(sizeof(T) == 1)
    {
        return __builtin_clz(static_cast<uint32_t>(src)) - 24;
    } else
    {
        int count = 0;
        for(int i = total_bits - 1; i >= 0; --i)
        {
            if((src >> i) & 1)
                break;

            ++count;
        }
        return count;
    }
#elif defined(_MSC_VER)
    if constexpr(sizeof(T) == 8)
    {
        unsigned long index = 0;
        _BitScanReverse64(&index, static_cast<uint64_t>(src));
        return 63 - static_cast<int>(index);
    } else if constexpr(sizeof(T) <= 4)
    {
        unsigned long index = 0;
        _BitScanReverse(&index, static_cast<uint32_t>(src));
        return (total_bits - 1) - static_cast<int>(index);
    } else
    {
        int count = 0;
        for(int i = total_bits - 1; i >= 0; --i)
        {
            if((src >> i) & 1)
                break;

            ++count;
        }
        return count;
    }
#else
    int count = 0;
    for(int i = total_bits - 1; i >= 0; --i)
    {
        if((src >> i) & 1)
            break;

        ++count;
    }
    return count;
#endif

#endif // HJ_BITS_HAS_STD_BIT
}

} // namespace hj::bits

#endif // BITS_HPP