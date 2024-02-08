#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

namespace libcpp
{

static const int endian_tag = 1;

inline bool is_big_endian()
{
    return *(char*)(&endian_tag) == 1;
}

class big_endian
{
public:
    big_endian() {}
    ~big_endian() {}

    template<typename T>
    T operator<<(const T n)
    {
        return libcpp::big_endian::covert(n);
    }

    static uint16_t covert(const uint16_t n)
    {
        return htons(n);
    }

    static int16_t covert(const int16_t n)
    {
        return (int16_t)(htons(n));
    }

    static uint32_t covert(const uint32_t n)
    {
        return htonl(n);
    }

    static int32_t covert(const int32_t n)
    {
        return (int32_t)(htonl(n));
    }

    static uint64_t covert(const uint64_t n)
    {
        return (((uint64_t) htonl(n)) << 32) + htonl(n >> 32);
    }

    static int64_t covert(const int64_t n)
    {
        return (((int64_t) htonl(n)) << 32) + (htonl(n >> 32));
    }
};
static libcpp::big_endian big;

class little_endian
{
public:
    little_endian() {}
    ~little_endian() {}

    template<typename T>
    T operator<<(const T n)
    {
        return libcpp::little_endian::covert(n);
    }

    static uint16_t covert(const uint16_t n)
    {
        return ntohs(n);
    }

    static int16_t covert(const int16_t n)
    {
        return (int16_t)(ntohs(n));
    }

    static uint32_t covert(const uint32_t n)
    {
        return ntohl(n);
    }

    static int32_t covert(const int32_t n)
    {
        return (int32_t)(ntohl(n));
    }

    static uint64_t covert(const uint64_t n)
    {
        return (((uint64_t) ntohl(n)) << 32) + htonl(n >> 32);
    }

    static int64_t covert(const int64_t n)
    {
        return (((int64_t) ntohl(n)) << 32) + (htonl(n >> 32));
    }
};
static libcpp::little_endian little;

}

#endif