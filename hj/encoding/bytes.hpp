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

#ifndef BYTES_HPP
#define BYTES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace hj
{

// ============================================================================
// IEEE-754 Binary32 / Binary64 Standard Compliance Asserts
// ============================================================================

static_assert(sizeof(float) == 4, "float must be 32 bits (IEEE-754 binary32)");
static_assert(sizeof(double) == 8,
              "double must be 64 bits (IEEE-754 binary64)");
static_assert(std::numeric_limits<float>::is_iec559,
              "float must comply with IEC 559 / IEEE 754 standard");
static_assert(std::numeric_limits<double>::is_iec559,
              "double must comply with IEC 559 / IEEE 754 standard");

// ============================================================================
// Byte-like Type Constraint Traits
// ============================================================================

template <typename T>
struct is_byte_like : std::false_type
{
};

template <>
struct is_byte_like<char> : std::true_type
{
};
template <>
struct is_byte_like<unsigned char> : std::true_type
{
};
template <>
struct is_byte_like<signed char> : std::true_type
{
};
template <>
struct is_byte_like<std::byte> : std::true_type
{
};

template <typename T>
inline constexpr bool is_byte_like_v = is_byte_like<std::remove_cv_t<T>>::value;

// ============================================================================
// C++17 Byte Buffer Views
// ============================================================================

/// 只读字节内存视图 (类似 C++20 std::span<const uint8_t>)
class byte_view
{
  public:
    constexpr byte_view() noexcept
        : _data(nullptr)
        , _size(0)
    {
    }

    constexpr byte_view(const uint8_t *data, std::size_t size) noexcept
        : _data(data)
        , _size(size)
    {
    }

    byte_view(const char *data, std::size_t size) noexcept
        : _data(reinterpret_cast<const uint8_t *>(data))
        , _size(size)
    {
    }

    byte_view(const std::byte *data, std::size_t size) noexcept
        : _data(reinterpret_cast<const uint8_t *>(data))
        , _size(size)
    {
    }

    template <std::size_t N>
    constexpr byte_view(const std::array<uint8_t, N> &arr) noexcept
        : _data(arr.data())
        , _size(N)
    {
    }

    template <std::size_t N>
    constexpr byte_view(const std::array<std::byte, N> &arr) noexcept
        : _data(reinterpret_cast<const uint8_t *>(arr.data()))
        , _size(N)
    {
    }

    template <std::size_t N>
    constexpr byte_view(const uint8_t (&arr)[N]) noexcept
        : _data(arr)
        , _size(N)
    {
    }

    template <std::size_t N>
    byte_view(const char (&arr)[N]) noexcept
        : _data(reinterpret_cast<const uint8_t *>(arr))
        , _size(N)
    {
    }

    template <std::size_t N>
    byte_view(const std::byte (&arr)[N]) noexcept
        : _data(reinterpret_cast<const uint8_t *>(arr))
        , _size(N)
    {
    }

    template <typename Container,
              typename = std::enable_if_t<
                  !std::is_same_v<std::decay_t<Container>, byte_view>
                  && !std::is_array_v<std::remove_reference_t<Container>>
                  && std::is_pointer_v<
                      decltype(std::declval<const Container &>().data())>
                  && is_byte_like_v<std::remove_pointer_t<
                      decltype(std::declval<const Container &>().data())>>>>
    byte_view(const Container &c) noexcept
        : _data(reinterpret_cast<const uint8_t *>(c.data()))
        , _size(c.size())
    {
    }

    [[nodiscard]] constexpr const uint8_t *data() const noexcept
    {
        return _data;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return _size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }
    constexpr const uint8_t     &operator[](std::size_t idx) const noexcept
    {
        return _data[idx];
    }

  private:
    const uint8_t *_data;
    std::size_t    _size;
};

/// 可写字节内存视图 (类似 C++20 std::span<uint8_t>)
class mutable_byte_view
{
  public:
    constexpr mutable_byte_view() noexcept
        : _data(nullptr)
        , _size(0)
    {
    }

    constexpr mutable_byte_view(uint8_t *data, std::size_t size) noexcept
        : _data(data)
        , _size(size)
    {
    }

    mutable_byte_view(char *data, std::size_t size) noexcept
        : _data(reinterpret_cast<uint8_t *>(data))
        , _size(size)
    {
    }

    mutable_byte_view(std::byte *data, std::size_t size) noexcept
        : _data(reinterpret_cast<uint8_t *>(data))
        , _size(size)
    {
    }

    template <std::size_t N>
    constexpr mutable_byte_view(std::array<uint8_t, N> &arr) noexcept
        : _data(arr.data())
        , _size(N)
    {
    }

    template <std::size_t N>
    constexpr mutable_byte_view(std::array<std::byte, N> &arr) noexcept
        : _data(reinterpret_cast<uint8_t *>(arr.data()))
        , _size(N)
    {
    }

    template <std::size_t N>
    constexpr mutable_byte_view(uint8_t (&arr)[N]) noexcept
        : _data(arr)
        , _size(N)
    {
    }

    template <std::size_t N>
    mutable_byte_view(char (&arr)[N]) noexcept
        : _data(reinterpret_cast<uint8_t *>(arr))
        , _size(N)
    {
    }

    template <std::size_t N>
    mutable_byte_view(std::byte (&arr)[N]) noexcept
        : _data(reinterpret_cast<uint8_t *>(arr))
        , _size(N)
    {
    }

    template <
        typename Container,
        typename = std::enable_if_t<
            !std::is_same_v<std::decay_t<Container>, mutable_byte_view>
            && !std::is_array_v<std::remove_reference_t<Container>>
            && std::is_pointer_v<decltype(std::declval<Container &>().data())>
            && is_byte_like_v<std::remove_pointer_t<
                decltype(std::declval<Container &>().data())>>>>
    mutable_byte_view(Container &c) noexcept
        : _data(reinterpret_cast<uint8_t *>(c.data()))
        , _size(c.size())
    {
    }

    [[nodiscard]] constexpr uint8_t    *data() const noexcept { return _data; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return _size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }
    constexpr uint8_t           &operator[](std::size_t idx) const noexcept
    {
        return _data[idx];
    }

    operator byte_view() const noexcept { return byte_view(_data, _size); }

  private:
    uint8_t    *_data;
    std::size_t _size;
};

// ============================================================================
// Safe API (以 std::optional / bool 为返回值)
// ============================================================================

// --- Primitive Uint32 / Uint64 接口 ---

inline std::optional<uint32_t>
try_bytes_to_uint32(byte_view bytes, bool big_endian = true) noexcept
{
    if(bytes.size() < 4)
        return std::nullopt;

    uint32_t n = 0;
    if(big_endian)
    {
        n |= (static_cast<uint32_t>(bytes[0]) << 24);
        n |= (static_cast<uint32_t>(bytes[1]) << 16);
        n |= (static_cast<uint32_t>(bytes[2]) << 8);
        n |= (static_cast<uint32_t>(bytes[3]));
    } else
    {
        n |= (static_cast<uint32_t>(bytes[3]) << 24);
        n |= (static_cast<uint32_t>(bytes[2]) << 16);
        n |= (static_cast<uint32_t>(bytes[1]) << 8);
        n |= (static_cast<uint32_t>(bytes[0]));
    }
    return n;
}

inline bool try_uint32_to_bytes(mutable_byte_view bytes,
                                uint32_t          val,
                                bool              big_endian = true) noexcept
{
    if(bytes.size() < 4)
        return false;

    if(big_endian)
    {
        bytes[0] = static_cast<uint8_t>((val >> 24) & 0xFF);
        bytes[1] = static_cast<uint8_t>((val >> 16) & 0xFF);
        bytes[2] = static_cast<uint8_t>((val >> 8) & 0xFF);
        bytes[3] = static_cast<uint8_t>(val & 0xFF);
    } else
    {
        bytes[3] = static_cast<uint8_t>((val >> 24) & 0xFF);
        bytes[2] = static_cast<uint8_t>((val >> 16) & 0xFF);
        bytes[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
        bytes[0] = static_cast<uint8_t>(val & 0xFF);
    }
    return true;
}

inline std::optional<uint64_t>
try_bytes_to_uint64(byte_view bytes, bool big_endian = true) noexcept
{
    if(bytes.size() < 8)
        return std::nullopt;

    uint64_t n = 0;
    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            n |= (static_cast<uint64_t>(bytes[i]) << (56 - 8 * i));
    } else
    {
        for(int i = 0; i < 8; ++i)
            n |= (static_cast<uint64_t>(bytes[i]) << (8 * i));
    }
    return n;
}

inline bool try_uint64_to_bytes(mutable_byte_view bytes,
                                uint64_t          val,
                                bool              big_endian = true) noexcept
{
    if(bytes.size() < 8)
        return false;

    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = static_cast<uint8_t>((val >> (56 - 8 * i)) & 0xFF);
    } else
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = static_cast<uint8_t>((val >> (8 * i)) & 0xFF);
    }
    return true;
}

// --- Bool 转换 ---

inline std::optional<bool> try_bytes_to_bool(byte_view bytes) noexcept
{
    if(bytes.size() < 1)
        return std::nullopt;

    switch(bytes[0])
    {
        case 0x00:
            return false;
        case 0x01:
            return true;
        default:
            return std::nullopt;
    }
}

inline bool try_bool_to_bytes(mutable_byte_view bytes, bool b) noexcept
{
    if(bytes.size() < 1)
        return false;

    bytes[0] = b ? 0x01 : 0x00;
    return true;
}

// --- Int32 / Int64 转换 ---

inline std::optional<int32_t>
try_bytes_to_int32(byte_view bytes, bool big_endian = true) noexcept
{
    auto u = try_bytes_to_uint32(bytes, big_endian);
    if(!u)
        return std::nullopt;

    int32_t result;
    std::memcpy(&result, &*u, sizeof(result));
    return result;
}

inline bool try_int32_to_bytes(mutable_byte_view bytes,
                               int32_t           n,
                               bool              big_endian = true) noexcept
{
    uint32_t u;
    std::memcpy(&u, &n, sizeof(u));
    return try_uint32_to_bytes(bytes, u, big_endian);
}

inline std::optional<int64_t>
try_bytes_to_int64(byte_view bytes, bool big_endian = true) noexcept
{
    auto u = try_bytes_to_uint64(bytes, big_endian);
    if(!u)
        return std::nullopt;

    int64_t result;
    std::memcpy(&result, &*u, sizeof(result));
    return result;
}

inline bool try_int64_to_bytes(mutable_byte_view bytes,
                               int64_t           n,
                               bool              big_endian = true) noexcept
{
    uint64_t u;
    std::memcpy(&u, &n, sizeof(u));
    return try_uint64_to_bytes(bytes, u, big_endian);
}

// --- Float / Double 转换 ---

inline std::optional<float> try_bytes_to_float(byte_view bytes,
                                               bool big_endian = true) noexcept
{
    auto u = try_bytes_to_uint32(bytes, big_endian);
    if(!u)
        return std::nullopt;

    float f;
    std::memcpy(&f, &*u, sizeof(float));
    return f;
}

inline bool try_float_to_bytes(mutable_byte_view bytes,
                               float             f,
                               bool              big_endian = true) noexcept
{
    uint32_t u;
    std::memcpy(&u, &f, sizeof(uint32_t));
    return try_uint32_to_bytes(bytes, u, big_endian);
}

inline std::optional<double>
try_bytes_to_double(byte_view bytes, bool big_endian = true) noexcept
{
    auto u = try_bytes_to_uint64(bytes, big_endian);
    if(!u)
        return std::nullopt;

    double d;
    std::memcpy(&d, &*u, sizeof(double));
    return d;
}

inline bool try_double_to_bytes(mutable_byte_view bytes,
                                double            d,
                                bool              big_endian = true) noexcept
{
    uint64_t u;
    std::memcpy(&u, &d, sizeof(uint64_t));
    return try_uint64_to_bytes(bytes, u, big_endian);
}

// --- String 转换 ---

/// 严格非截断模式：必须全量写入，容量不足直接返回 false
inline bool try_string_to_bytes(mutable_byte_view bytes,
                                std::string_view  str) noexcept
{
    if(bytes.size() < str.size())
        return false;

    if(!str.empty())
    {
        std::memcpy(bytes.data(), str.data(), str.size());
    }
    return true;
}

/// 尽可能写入模式：发生溢出时进行截断，返回实际复制的字节数
inline std::size_t string_to_bytes(mutable_byte_view bytes,
                                   std::string_view  str) noexcept
{
    const std::size_t copy_len = (std::min) (bytes.size(), str.size());
    if(copy_len > 0)
    {
        std::memcpy(bytes.data(), str.data(), copy_len);
    }
    return copy_len;
}

inline std::string bytes_to_string(byte_view bytes, std::size_t sz)
{
    const std::size_t len = (std::min) (bytes.size(), sz);
    return std::string(reinterpret_cast<const char *>(bytes.data()), len);
}

// ============================================================================
// Contract API / 便捷重载
// ============================================================================

inline bool bytes_to_bool(byte_view bytes)
{
    auto res = try_bytes_to_bool(bytes);
    assert(res.has_value()
           && "Precondition violated: bytes size >= 1 and byte must be 0x00 or "
              "0x01");
    return *res;
}

inline bool bytes_to_bool(const unsigned char *bytes, const std::size_t sz)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_bool(byte_view(bytes, sz));
}

inline bool bytes_to_bool(const std::byte *bytes, const std::size_t sz)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_bool(byte_view(bytes, sz));
}

template <typename T>
inline T &bool_to_bytes(T &bytes, const bool b)
{
    mutable_byte_view v(bytes);
    bool              ok = try_bool_to_bytes(v, b);
    assert(
        ok
        && "Precondition violated: destination buffer size must be at least 1");
    (void) ok;
    return bytes;
}

inline unsigned char *
bool_to_bytes(unsigned char *bytes, std::size_t &sz, const bool b)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_bool_to_bytes(mutable_byte_view(bytes, sz), b))
        return nullptr;

    sz = 1;
    return bytes;
}

inline std::byte *bool_to_bytes(std::byte *bytes, std::size_t &sz, const bool b)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_bool_to_bytes(mutable_byte_view(bytes, sz), b))
        return nullptr;

    sz = 1;
    return bytes;
}

inline int32_t bytes_to_int32(byte_view bytes, bool big_endian = true)
{
    auto res = try_bytes_to_int32(bytes, big_endian);
    assert(res.has_value()
           && "Precondition violated: bytes size must be at least 4");
    return *res;
}

inline int32_t bytes_to_int32(const unsigned char *bytes,
                              const std::size_t    sz,
                              bool                 big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_int32(byte_view(bytes, sz), big_endian);
}

inline int32_t bytes_to_int32(const std::byte  *bytes,
                              const std::size_t sz,
                              bool              big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_int32(byte_view(bytes, sz), big_endian);
}

template <typename T>
inline T &int32_to_bytes(T &bytes, const int32_t n, bool big_endian = true)
{
    mutable_byte_view v(bytes);
    bool              ok = try_int32_to_bytes(v, n, big_endian);
    assert(
        ok
        && "Precondition violated: destination buffer size must be at least 4");
    (void) ok;
    return bytes;
}

inline unsigned char *int32_to_bytes(unsigned char *bytes,
                                     std::size_t   &sz,
                                     int32_t        n,
                                     bool           big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_int32_to_bytes(mutable_byte_view(bytes, sz), n, big_endian))
        return nullptr;

    sz = 4;
    return bytes;
}

inline std::byte *int32_to_bytes(std::byte   *bytes,
                                 std::size_t &sz,
                                 int32_t      n,
                                 bool         big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_int32_to_bytes(mutable_byte_view(bytes, sz), n, big_endian))
        return nullptr;

    sz = 4;
    return bytes;
}

inline int64_t bytes_to_int64(byte_view bytes, bool big_endian = true)
{
    auto res = try_bytes_to_int64(bytes, big_endian);
    assert(res.has_value()
           && "Precondition violated: bytes size must be at least 8");
    return *res;
}

inline int64_t bytes_to_int64(const unsigned char *bytes,
                              const std::size_t    sz,
                              bool                 big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_int64(byte_view(bytes, sz), big_endian);
}

inline int64_t bytes_to_int64(const std::byte  *bytes,
                              const std::size_t sz,
                              bool              big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_int64(byte_view(bytes, sz), big_endian);
}

template <typename T>
inline T &int64_to_bytes(T &bytes, const int64_t n, bool big_endian = true)
{
    mutable_byte_view v(bytes);
    bool              ok = try_int64_to_bytes(v, n, big_endian);
    assert(
        ok
        && "Precondition violated: destination buffer size must be at least 8");
    (void) ok;
    return bytes;
}

inline unsigned char *int64_to_bytes(unsigned char *bytes,
                                     std::size_t   &sz,
                                     const int64_t  n,
                                     bool           big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_int64_to_bytes(mutable_byte_view(bytes, sz), n, big_endian))
        return nullptr;

    sz = 8;
    return bytes;
}

inline std::byte *int64_to_bytes(std::byte    *bytes,
                                 std::size_t  &sz,
                                 const int64_t n,
                                 bool          big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_int64_to_bytes(mutable_byte_view(bytes, sz), n, big_endian))
        return nullptr;

    sz = 8;
    return bytes;
}

inline float bytes_to_float(byte_view bytes, bool big_endian = true)
{
    auto res = try_bytes_to_float(bytes, big_endian);
    assert(res.has_value()
           && "Precondition violated: bytes size must be at least 4");
    return *res;
}

inline float bytes_to_float(const unsigned char *bytes,
                            const std::size_t    sz,
                            bool                 big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_float(byte_view(bytes, sz), big_endian);
}

inline float bytes_to_float(const std::byte  *bytes,
                            const std::size_t sz,
                            bool              big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_float(byte_view(bytes, sz), big_endian);
}

template <typename T>
inline T &float_to_bytes(T &bytes, const float f, bool big_endian = true)
{
    mutable_byte_view v(bytes);
    bool              ok = try_float_to_bytes(v, f, big_endian);
    assert(
        ok
        && "Precondition violated: destination buffer size must be at least 4");
    (void) ok;
    return bytes;
}

inline unsigned char *float_to_bytes(unsigned char *bytes,
                                     std::size_t   &sz,
                                     const float    f,
                                     bool           big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_float_to_bytes(mutable_byte_view(bytes, sz), f, big_endian))
        return nullptr;

    sz = sizeof(float);
    return bytes;
}

inline std::byte *float_to_bytes(std::byte   *bytes,
                                 std::size_t &sz,
                                 const float  f,
                                 bool         big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_float_to_bytes(mutable_byte_view(bytes, sz), f, big_endian))
        return nullptr;

    sz = sizeof(float);
    return bytes;
}

inline double bytes_to_double(byte_view bytes, bool big_endian = true)
{
    auto res = try_bytes_to_double(bytes, big_endian);
    assert(res.has_value()
           && "Precondition violated: bytes size must be at least 8");
    return *res;
}

inline double bytes_to_double(const unsigned char *bytes,
                              const std::size_t    sz,
                              bool                 big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_double(byte_view(bytes, sz), big_endian);
}

inline double bytes_to_double(const std::byte  *bytes,
                              const std::size_t sz,
                              bool              big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    return bytes_to_double(byte_view(bytes, sz), big_endian);
}

template <typename T>
inline T &double_to_bytes(T &bytes, const double d, bool big_endian = true)
{
    mutable_byte_view v(bytes);
    bool              ok = try_double_to_bytes(v, d, big_endian);
    assert(
        ok
        && "Precondition violated: destination buffer size must be at least 8");
    (void) ok;
    return bytes;
}

inline unsigned char *double_to_bytes(unsigned char *bytes,
                                      std::size_t   &sz,
                                      const double   d,
                                      bool           big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_double_to_bytes(mutable_byte_view(bytes, sz), d, big_endian))
        return nullptr;

    sz = sizeof(double);
    return bytes;
}

inline std::byte *double_to_bytes(std::byte   *bytes,
                                  std::size_t &sz,
                                  const double d,
                                  bool         big_endian = true)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    if(!try_double_to_bytes(mutable_byte_view(bytes, sz), d, big_endian))
        return nullptr;

    sz = sizeof(double);
    return bytes;
}

template <typename T>
inline std::string bytes_to_string(const T &bytes, std::size_t sz)
{
    return bytes_to_string(byte_view(bytes), sz);
}

inline std::string bytes_to_string(const unsigned char *bytes, std::size_t sz)
{
    return bytes_to_string(byte_view(bytes, sz), sz);
}

inline std::string bytes_to_string(const std::byte *bytes, std::size_t sz)
{
    return bytes_to_string(byte_view(bytes, sz), sz);
}

template <typename T>
inline T &string_to_bytes(T &bytes, const std::string &str)
{
    mutable_byte_view v(bytes);
    string_to_bytes(v, std::string_view(str));
    return bytes;
}

inline unsigned char *
string_to_bytes(unsigned char *bytes, std::size_t &sz, const std::string &str)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    sz = string_to_bytes(mutable_byte_view(bytes, sz), std::string_view(str));
    return bytes;
}

inline std::byte *
string_to_bytes(std::byte *bytes, std::size_t &sz, const std::string &str)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    sz = string_to_bytes(mutable_byte_view(bytes, sz), std::string_view(str));
    return bytes;
}

inline char *
string_to_bytes(char *bytes, std::size_t &sz, const std::string &str)
{
    assert(bytes != nullptr
           && "Precondition violated: bytes pointer must not be null");
    sz = string_to_bytes(mutable_byte_view(bytes, sz), std::string_view(str));
    return bytes;
}

} // namespace hj

#endif // BYTES_HPP