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

#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <cstdint>
#include <type_traits>

namespace hj
{

namespace detail
{

template <typename T>
struct is_valid_endian_type
{
    using CleanT                = std::remove_cv_t<T>;
    static constexpr bool value = std::is_same_v<CleanT, std::int8_t>
                                  || std::is_same_v<CleanT, std::uint8_t>
                                  || std::is_same_v<CleanT, std::int16_t>
                                  || std::is_same_v<CleanT, std::uint16_t>
                                  || std::is_same_v<CleanT, std::int32_t>
                                  || std::is_same_v<CleanT, std::uint32_t>
                                  || std::is_same_v<CleanT, std::int64_t>
                                  || std::is_same_v<CleanT, std::uint64_t>;
};

template <typename T>
inline constexpr bool is_valid_endian_type_v = is_valid_endian_type<T>::value;

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
    return false;
#endif
}

inline constexpr uint16_t bswap16(uint16_t x) noexcept
{
    return static_cast<uint16_t>((x >> 8) | (x << 8));
}

inline constexpr uint32_t bswap32(uint32_t x) noexcept
{
    return ((x >> 24) & 0x000000FFu) | ((x >> 8) & 0x0000FF00u)
           | ((x << 8) & 0x00FF0000u) | ((x << 24) & 0xFF000000u);
}

inline constexpr uint64_t bswap64(uint64_t x) noexcept
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

template <typename T>
inline constexpr T byte_swap(T val) noexcept
{
    static_assert(is_valid_endian_type_v<T>,
                  "T must be an explicit width integer type");

    if constexpr(sizeof(T) == 1)
        return val;

    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT u_val = static_cast<UnsignedT>(val);

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

    return static_cast<T>(u_val);
}

} // namespace detail

inline constexpr bool is_big_endian() noexcept
{
    return detail::is_big_endian_impl();
}

template <typename T>
inline constexpr T to_big_endian(T v) noexcept
{
    static_assert(detail::is_valid_endian_type_v<T>,
                  "T must be an explicit width integer type");

    if constexpr(!detail::is_big_endian_impl())
        return detail::byte_swap(v);

    return v;
}

template <typename T>
inline constexpr T from_big_endian(T v) noexcept
{
    return to_big_endian(v);
}

template <typename T>
inline constexpr T to_little_endian(T v) noexcept
{
    static_assert(detail::is_valid_endian_type_v<T>,
                  "T must be an explicit width integer type");

    if constexpr(detail::is_big_endian_impl())
        return detail::byte_swap(v);

    return v;
}

template <typename T>
inline constexpr T from_little_endian(T v) noexcept
{
    return to_little_endian(v);
}

} // namespace hj

#endif // ENDIAN_HPP