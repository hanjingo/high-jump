#ifndef BYTES_HPP
#define BYTES_HPP

#include <cstring>
#include <string>
#include <type_traits>
#include <array>

namespace hj
{

template <typename T>
inline bool bytes_to_bool(const T &bytes)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    return (bytes[0] & 0x01) == 1;
}

template <typename T>
inline T &bool_to_bytes(bool b, T &bytes)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    bytes[0] = b ? 0x01 : 0x00;
    return bytes;
}

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

template <typename T>
inline T &int32_to_bytes(int32_t n, T &bytes, bool big_endian = true)
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

template <typename T>
inline T &int64_to_bytes(int64_t n, T &bytes, bool big_endian = true)
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


template <typename T>
inline float bytes_to_float(const T &bytes)
{
    static_assert(sizeof(T) >= sizeof(float), "T must have at least 4 bytes");
    float f;
    std::memcpy(&f, bytes.data(), sizeof(float));
    return f;
}


template <typename T>
inline T &float_to_bytes(float f, T &bytes)
{
    static_assert(sizeof(T) >= sizeof(float), "T must have at least 4 bytes");
    std::memcpy(bytes.data(), &f, sizeof(float));
    return bytes;
}


template <typename T>
inline double bytes_to_double(const T &bytes)
{
    static_assert(sizeof(T) >= sizeof(double), "T must have at least 8 bytes");
    double d;
    std::memcpy(&d, bytes.data(), sizeof(double));
    return d;
}


template <typename T>
inline T &double_to_bytes(double d, T &bytes)
{
    static_assert(sizeof(T) >= sizeof(double), "T must have at least 8 bytes");
    std::memcpy(bytes.data(), &d, sizeof(double));
    return bytes;
}


template <typename T>
inline std::string bytes_to_string(const T &bytes, size_t sz)
{
    return std::string(reinterpret_cast<const char *>(bytes.data()), sz);
}


template <typename T>
inline T &string_to_bytes(const std::string &str, T &bytes)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable");
    std::memcpy(bytes.data(), str.data(), std::min(str.size(), bytes.size()));
    return bytes;
}

} // namespace hj

#endif