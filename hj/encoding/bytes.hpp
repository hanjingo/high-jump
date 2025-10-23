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

#include <cstring>
#include <string>
#include <type_traits>
#include <array>
#include <algorithm>
#include <cassert>

namespace hj
{

// Convert bytes to/from basic types
template <typename T>
inline bool bytes_to_bool(const T &bytes)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    return (bytes[0] & 0x01) == 1;
}

inline bool bytes_to_bool(const unsigned char *bytes, const size_t sz)
{
    assert(
        bytes != nullptr && sz >= 1
        && "bytes pointer must not be null and size must be at least 1 byte");
    return (bytes[0] & 0x01) == 1;
}

template <typename T>
inline T &bool_to_bytes(T &bytes, const bool b)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    bytes[0] = b ? 0x01 : 0x00;
    return bytes;
}

inline unsigned char *
bool_to_bytes(unsigned char *bytes, size_t &sz, const bool b)
{
    assert(
        bytes != nullptr && sz >= 1
        && "bytes pointer must not be null and size must be at least 1 byte");
    sz       = 1;
    bytes[0] = b ? 0x01 : 0x00;
    return bytes;
}


// Convert bytes to/from int32
template <typename T>
inline int32_t bytes_to_int32(const T &bytes, bool big_endian = true)
{
    static_assert(sizeof(T) >= 4, "T must have at least 4 bytes");
    uint32_t n = 0;
    if(big_endian)
    {
        n |= (uint32_t(bytes[0]) << 24);
        n |= (uint32_t(bytes[1]) << 16);
        n |= (uint32_t(bytes[2]) << 8);
        n |= (uint32_t(bytes[3]));
    } else
    {
        n |= (uint32_t(bytes[3]) << 24);
        n |= (uint32_t(bytes[2]) << 16);
        n |= (uint32_t(bytes[1]) << 8);
        n |= (uint32_t(bytes[0]));
    }
    return static_cast<int32_t>(n);
}

inline int32_t bytes_to_int32(const unsigned char *bytes,
                              const size_t         sz,
                              bool                 big_endian = true)
{
    assert(
        bytes != nullptr && sz >= 4
        && "bytes pointer must not be null and size must be at least 4 bytes");
    uint32_t n = 0;
    if(big_endian)
    {
        n |= (uint32_t(bytes[0]) << 24);
        n |= (uint32_t(bytes[1]) << 16);
        n |= (uint32_t(bytes[2]) << 8);
        n |= (uint32_t(bytes[3]));
    } else
    {
        n |= (uint32_t(bytes[3]) << 24);
        n |= (uint32_t(bytes[2]) << 16);
        n |= (uint32_t(bytes[1]) << 8);
        n |= (uint32_t(bytes[0]));
    }
    return static_cast<int32_t>(n);
}

template <typename T>
inline T &int32_to_bytes(T &bytes, const int32_t n, bool big_endian = true)
{
    static_assert(sizeof(T) >= 4, "T must have at least 4 bytes");
    if(big_endian)
    {
        bytes[0] = (n >> 24) & 0xFF;
        bytes[1] = (n >> 16) & 0xFF;
        bytes[2] = (n >> 8) & 0xFF;
        bytes[3] = n & 0xFF;
    } else
    {
        bytes[3] = (n >> 24) & 0xFF;
        bytes[2] = (n >> 16) & 0xFF;
        bytes[1] = (n >> 8) & 0xFF;
        bytes[0] = n & 0xFF;
    }
    return bytes;
}

inline unsigned char *int32_to_bytes(unsigned char *bytes,
                                     size_t        &sz,
                                     int32_t        n,
                                     bool           big_endian = true)
{
    assert(
        bytes != nullptr && sz >= 4
        && "bytes pointer must not be null and size must be at least 4 bytes");
    sz = 4;
    if(big_endian)
    {
        bytes[0] = (n >> 24) & 0xFF;
        bytes[1] = (n >> 16) & 0xFF;
        bytes[2] = (n >> 8) & 0xFF;
        bytes[3] = n & 0xFF;
    } else
    {
        bytes[3] = (n >> 24) & 0xFF;
        bytes[2] = (n >> 16) & 0xFF;
        bytes[1] = (n >> 8) & 0xFF;
        bytes[0] = n & 0xFF;
    }
    return bytes;
}


// Convert bytes to/from int64
template <typename T>
inline int64_t bytes_to_int64(const T &bytes, bool big_endian = true)
{
    static_assert(sizeof(T) >= 8, "T must have at least 8 bytes");
    uint64_t n = 0;
    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            n |= (uint64_t(bytes[i]) << (56 - 8 * i));
    } else
    {
        for(int i = 0; i < 8; ++i)
            n |= (uint64_t(bytes[i]) << (8 * i));
    }
    return static_cast<int64_t>(n);
}

inline int64_t bytes_to_int64(const unsigned char *bytes,
                              const size_t         sz,
                              bool                 big_endian = true)
{
    assert(sz >= 8 && "bytes must have at least 8 bytes");
    uint64_t n = 0;
    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            n |= (uint64_t(bytes[i]) << (56 - 8 * i));
    } else
    {
        for(int i = 0; i < 8; ++i)
            n |= (uint64_t(bytes[i]) << (8 * i));
    }
    return static_cast<int64_t>(n);
}

template <typename T>
inline T &int64_to_bytes(T &bytes, const int64_t n, bool big_endian = true)
{
    static_assert(sizeof(T) >= 8, "T must have at least 8 bytes");
    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = (n >> (56 - 8 * i)) & 0xFF;
    } else
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = (n >> (8 * i)) & 0xFF;
    }
    return bytes;
}

inline unsigned char *int64_to_bytes(unsigned char *bytes,
                                     size_t        &sz,
                                     const int64_t  n,
                                     bool           big_endian = true)
{
    assert(
        bytes != nullptr && sz >= 8
        && "bytes pointer must not be null and size must be at least 8 bytes");
    sz = 8;
    if(big_endian)
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = (n >> (56 - 8 * i)) & 0xFF;
    } else
    {
        for(int i = 0; i < 8; ++i)
            bytes[i] = (n >> (8 * i)) & 0xFF;
    }
    return bytes;
}


// Convert bytes to/from float
template <typename T>
inline float bytes_to_float(const T &bytes)
{
    static_assert(sizeof(T) >= sizeof(float), "T must have at least 4 bytes");
    float f;
    std::memcpy(&f, bytes.data(), sizeof(float));
    return f;
}

inline float bytes_to_float(const unsigned char *bytes, const size_t sz)
{
    assert(sz >= sizeof(float) && "bytes must have at least 4 bytes");
    float f;
    std::memcpy(&f, bytes, sizeof(float));
    return f;
}

template <typename T>
inline T &float_to_bytes(T &bytes, const float f)
{
    static_assert(sizeof(T) >= sizeof(float), "T must have at least 4 bytes");
    std::memcpy(bytes.data(), &f, sizeof(float));
    return bytes;
}

inline unsigned char *
float_to_bytes(unsigned char *bytes, size_t &sz, const float f)
{
    assert(
        bytes != nullptr && sz >= sizeof(float)
        && "bytes pointer must not be null and size must be at least 4 bytes");
    sz = sizeof(float);
    std::memcpy(bytes, &f, sz);
    return bytes;
}


// Convert bytes to/from double
template <typename T>
inline double bytes_to_double(const T &bytes)
{
    static_assert(sizeof(T) >= sizeof(double), "T must have at least 8 bytes");
    double d;
    std::memcpy(&d, bytes.data(), sizeof(double));
    return d;
}

inline double bytes_to_double(const unsigned char *bytes, const size_t sz)
{
    assert(sz >= sizeof(double) && "T must have at least 8 bytes");
    double d;
    std::memcpy(&d, bytes, sizeof(double));
    return d;
}

template <typename T>
inline T &double_to_bytes(T &bytes, const double d)
{
    static_assert(sizeof(T) >= sizeof(double), "T must have at least 8 bytes");
    std::memcpy(bytes.data(), &d, sizeof(double));
    return bytes;
}

inline unsigned char *
double_to_bytes(unsigned char *bytes, size_t &sz, const double d)
{
    assert(
        bytes != nullptr && sz >= sizeof(double)
        && "bytes pointer must not be null and size must be at least 8 bytes");
    sz = sizeof(double);
    std::memcpy(bytes, &d, sz);
    return bytes;
}


// Convert bytes to/from string
template <typename T>
inline std::string bytes_to_string(const T &bytes, const size_t sz)
{
    return std::string(reinterpret_cast<const char *>(bytes.data()), sz);
}

inline std::string bytes_to_string(const unsigned char *bytes, const size_t sz)
{
    return std::string(reinterpret_cast<const char *>(bytes), sz);
}

inline std::string bytes_to_string(const char *bytes, const size_t sz)
{
    return std::string(bytes, sz);
}

template <typename T>
inline T &string_to_bytes(T &bytes, const std::string &str)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    std::memcpy(bytes.data(), str.data(), std::min(str.size(), bytes.size()));
    return bytes;
}

inline unsigned char *
string_to_bytes(unsigned char *bytes, size_t &sz, const std::string &str)
{
    assert(bytes != nullptr && sz >= str.size()
           && "bytes pointer must not be null and size must be at least string "
              "size");
    sz = std::min(str.size(), sz);
    std::memcpy(bytes, str.data(), sz);
    return bytes;
}

inline char *string_to_bytes(char *bytes, size_t &sz, const std::string &str)
{
    assert(bytes != nullptr && sz >= str.size()
           && "bytes pointer must not be null and size must be at least string "
              "size");
    sz = std::min(str.size(), sz);
    std::memcpy(bytes, str.data(), sz);
    return bytes;
}

} // namespace hj

#endif