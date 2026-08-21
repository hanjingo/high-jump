/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  Licensed under the GNU General Public License, Version 3.0.
 */

#ifndef REFLECT_HPP
#define REFLECT_HPP

#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <cstring>
#include <cstddef>
#include <stdexcept>

#include <boost/core/demangle.hpp>

namespace hj::reflect
{
template <typename T>
inline constexpr bool is_simple_v =
    std::is_trivial_v<T> && std::is_standard_layout_v<T>;

template <typename T>
constexpr bool is_simple(const T &) noexcept
{
    return is_simple_v<T>;
}

template <typename T>
[[deprecated("Use is_simple instead")]]
constexpr bool is_pod(const T &t) noexcept
{
    return is_simple(t);
}

template <typename T>
std::string type_name()
{
#if defined(__GNUC__) || defined(__clang__)
    return boost::core::demangle(typeid(T).name());
#else
    return typeid(T).name();
#endif
}

template <typename T>
std::string type_name(const T &)
{
    return type_name<T>();
}

template <typename T>
T copy(const T &t)
{
    return t;
}

template <typename T>
T clone(const T &t)
{
    return T(t);
}

/**
 * @brief Performs a raw binary dump of a trivially copyable type into std::byte.
 * 
 * WARNING: This is NOT a portable serialization format.
 * The serialized representation is implementation-defined, platform/compiler-dependent,
 * and includes padding bytes. It must NOT be used as a cross-platform wire format 
 * or long-term persistent storage format.
 */
template <typename T>
std::vector<std::byte> dump_binary(const T &t)
{
    static_assert(std::is_trivially_copyable_v<T>,
                  "Type must be trivially copyable for binary dump");
    const auto *ptr = reinterpret_cast<const std::byte *>(&t);
    return std::vector<std::byte>(ptr, ptr + sizeof(T));
}

/**
 * @brief Restores a type from a raw byte pointer and size with bounds checking.
 * 
 * WARNING: See dump_binary() for limitations.
 */
template <typename T>
T load_binary(const std::byte *buf, std::size_t size)
{
    static_assert(std::is_trivially_copyable_v<T>,
                  "Type must be trivially copyable for binary load");
    if(buf == nullptr || size < sizeof(T))
    {
        throw std::invalid_argument(
            "load_binary: buffer is null or size is smaller than type size");
    }
    T t;
    std::memcpy(&t, buf, sizeof(T));
    return t;
}

template <typename T>
T load_binary(const unsigned char *buf, std::size_t size)
{
    return load_binary<T>(reinterpret_cast<const std::byte *>(buf), size);
}

template <typename T>
[[deprecated("Use dump_binary instead")]]
std::vector<unsigned char> serialize(const T &t)
{
    auto        bytes = dump_binary(t);
    const auto *ptr   = reinterpret_cast<const unsigned char *>(bytes.data());
    return std::vector<unsigned char>(ptr, ptr + bytes.size());
}

template <typename T>
[[deprecated("Use load_binary with size check instead")]]
T unserialize(const unsigned char *buf)
{
    return load_binary<T>(buf, sizeof(T));
}

template <typename T>
[[deprecated("Use load_binary with size check instead")]]
T unserialize(const std::byte *buf)
{
    return load_binary<T>(buf, sizeof(T));
}

template <typename T, typename Member>
constexpr std::size_t offset_of(Member T::*member) noexcept
{
    static_assert(
        std::is_standard_layout_v<T>,
        "Type must be a standard-layout type for offset_of computation");
    return reinterpret_cast<std::size_t>(&(static_cast<T *>(nullptr)->*member));
}

template <typename T, typename Member>
constexpr std::size_t size_of(Member T::*member) noexcept
{
    static_assert(
        std::is_standard_layout_v<T>,
        "Type must be a standard-layout type for size_of computation");
    return sizeof(static_cast<T *>(nullptr)->*member);
}

template <typename T, typename Member>
constexpr std::size_t align_of(Member T::*) noexcept
{
    return alignof(Member);
}

template <typename T, typename... Others>
constexpr bool is_same_type(const T &, const Others &...) noexcept
{
    return (std::is_same_v<T, Others> && ...);
}

} // namespace hj::reflect

#endif