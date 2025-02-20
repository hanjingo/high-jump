#ifndef BYTES_HPP
#define BYTES_HPP

#include <string.h>
#include <string>

namespace libcpp
{
    
template<typename T>
extern bool bytes_to_bool(const T& bytes)
{
    return (bytes[0] & 0x01) == 1 ? true : false;
}

template<typename T>
extern T& bool_to_bytes(const bool b, T& bytes)
{
    if (b) {
        bytes[0] = (unsigned char)(0x01 & 1);
    } else {
        bytes[0] = (unsigned char)(0x01 & 0);
    }
    return bytes;
}

template<typename T>
extern int bytes_to_int(const T& bytes, bool big_endian = true)
{
    int n = 0;
    if (big_endian) {
        n  = bytes[3] & 0x000000FF;
        n |= ((bytes[2] << 8) & 0x0000FF00);
        n |= ((bytes[1] << 16) & 0x00FF0000);
        n |= ((bytes[0] << 24) & 0xFF000000);
    } else {
        n  = bytes[0] & 0x000000FF;
        n |= ((bytes[1] << 8) & 0x0000FF00);
        n |= ((bytes[2] << 16) & 0x00FF0000);
        n |= ((bytes[3] << 24) & 0xFF000000);
    }
    return n;
}

template<typename T>
extern T& int_to_bytes(const int n, T& bytes, bool big_endian = true)
{
    if (big_endian) {
        bytes[3] = (unsigned char)(0xFF & n);
        bytes[2] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[1] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[0] = (unsigned char)((0xFF000000 & n) >> 24);
    } else {
        bytes[0] = (unsigned char)(0xFF & n);
        bytes[1] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[2] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[3] = (unsigned char)((0xFF000000 & n) >> 24);
    }
    return bytes;
}

template<typename T>
extern long bytes_to_long(const T& bytes, bool big_endian = true)
{
    long n = 0;
    if (big_endian) {
        n  = bytes[3] & 0x000000FF;
        n |= ((bytes[2] << 8) & 0x0000FF00);
        n |= ((bytes[1] << 16) & 0x00FF0000);
        n |= ((bytes[0] << 24) & 0xFF000000);
    } else {
        n  = bytes[0] & 0xFF;
        n |= ((bytes[1] << 8) & 0xFF00);
        n |= ((bytes[2] << 16) & 0xFF0000);
        n |= ((bytes[3] << 24) & 0xFF000000);
    }
    return n;
}

template<typename T>
extern T& long_to_bytes(const long n, T& bytes, bool big_endian = true)
{
    if (big_endian) {
        bytes[3] = (unsigned char)(0xFF & n);
        bytes[2] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[1] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[0] = (unsigned char)((0xFF000000 & n) >> 24);
    } else {
        bytes[0] = (unsigned char)(0xFF & n);
        bytes[1] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[2] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[3] = (unsigned char)((0xFF000000 & n) >> 24);
    }

    return bytes;
}

template<typename T>
extern long long bytes_to_long_long(const T& bytes, bool big_endian = true)
{
    long long n = 0;
    if (big_endian) {
        n  = bytes[7] & 0xFF;
        n |= ((bytes[6] << 8) & 0xFF00);
        n |= ((bytes[5] << 16) & 0xFF0000);
        n |= ((bytes[4] << 24) & 0xFF000000);
        n |= ((((long long) bytes[3]) << 32) & 0xFF00000000);
        n |= ((((long long) bytes[2]) << 40) & 0xFF0000000000);
        n |= ((((long long) bytes[1]) << 48) & 0xFF000000000000);
        n |= ((((long long) bytes[0]) << 56) & 0xFF00000000000000);
    } else {
        n  = bytes[0] & 0xFF;
        n |= ((bytes[1] << 8) & 0xFF00);
        n |= ((bytes[2] << 16) & 0xFF0000);
        n |= ((bytes[3] << 24) & 0xFF000000);
        n |= ((((long long) bytes[4]) << 32) & 0xFF00000000);
        n |= ((((long long) bytes[5]) << 40) & 0xFF0000000000);
        n |= ((((long long) bytes[6]) << 48) & 0xFF000000000000);
        n |= ((((long long) bytes[7]) << 56) & 0xFF00000000000000);
    }
    return n;
}

template<typename T>
extern T& long_long_to_bytes(const long long n, T& bytes, bool big_endian = true)
{
    if (big_endian) {
        bytes[7] = (unsigned char)(0xFF & n);
        bytes[6] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[5] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[4] = (unsigned char)((0xFF000000 & n) >> 24);
        bytes[3] = (unsigned char)((0xFF00000000 & n) >> 32);
        bytes[2] = (unsigned char)((0xFF0000000000 & n) >> 40);
        bytes[1] = (unsigned char)((0xFF000000000000 & n) >> 48);
        bytes[0] = (unsigned char)((0xFF00000000000000 & n) >> 56);
    } else {
        bytes[0] = (unsigned char)(0xFF & n);
        bytes[1] = (unsigned char)((0xFF00 & n) >> 8);
        bytes[2] = (unsigned char)((0xFF0000 & n) >> 16);
        bytes[3] = (unsigned char)((0xFF000000 & n) >> 24);
        bytes[4] = (unsigned char)((0xFF00000000 & n) >> 32);
        bytes[5] = (unsigned char)((0xFF0000000000 & n) >> 40);
        bytes[6] = (unsigned char)((0xFF000000000000 & n) >> 48);
        bytes[7] = (unsigned char)((0xFF00000000000000 & n) >> 56);
    }
    return bytes;
}

template<typename T>
extern short bytes_to_short(const T& bytes, bool big_endian = true)
{
    short n = 0;
    if (big_endian) {
        n  = bytes[1] & 0xFF;
        n |= ((bytes[0] << 8) & 0xFF00);
    } else {
        n  = bytes[0] & 0xFF;
        n |= ((bytes[1] << 8) & 0xFF00);
    }
    return n;
}

template<typename T>
extern T& short_to_bytes(const short n, T& bytes, bool big_endian = true)
{
    if (big_endian) {
        bytes[1] = (unsigned char)(0xFF & n);
        bytes[0] = (unsigned char)((0xFF00 & n) >> 8);
    } else {
        bytes[0] = (unsigned char)(0xFF & n);
        bytes[1] = (unsigned char)((0xFF00 & n) >> 8);
    }
    return bytes;
}

template<typename T>
extern double bytes_to_double(const T& bytes)
{
    return *((double*)bytes);
}

template<typename T>
extern T& double_to_bytes(const double num, T& bytes)
{
    int i;
    std::size_t sz = sizeof(double);
    char* p = (char*)&num;
    for (i = 0; i < sz; i++) {
        bytes[i] = *p++;
    }
    return bytes;
}

template<typename T>
extern float bytes_to_float(const T& bytes)
{
    return *((float*)bytes);
}

template<typename T>
extern T& float_to_bytes(const float f, T& bytes)
{
    int i;
    size_t sz = sizeof(float);
    unsigned char* buf = (unsigned char*)&f;
    memcpy(bytes, buf, sz);
    return bytes;
}

template <typename T>
extern std::string& bytes_to_string(const T& bytes, const std::size_t sz, std::string& str)
{
    char buf[sz];
    memcpy(buf, bytes, sz);
    str = buf;
    return str;
}

template <typename T>
extern std::string bytes_to_string(const T& bytes, const std::size_t sz)
{
    char buf[sz];
    memcpy(buf, bytes, sz);
    return std::string(buf);
}

template <typename T>
extern T& string_to_bytes(const std::string& str, T& bytes)
{
    memcpy(bytes, str.c_str(), str.size());
    return bytes;
}

}

#endif