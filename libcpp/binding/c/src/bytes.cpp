#include <libcpp/encoding/bytes.hpp>

extern "C"
{
#include "bytes.h"
}

LIBCPP_API bool libcpp_bytes_to_bool(const unsigned char* bytes)
{
    return libcpp::bytes_to_bool(bytes);
}

LIBCPP_API unsigned char* libcpp_bool_to_bytes(const bool b,
                                               unsigned char* bytes)
{
    return libcpp::bool_to_bytes(b, bytes);
}

LIBCPP_API int libcpp_bytes_to_int(const unsigned char* bytes, bool big_endian)
{
    return libcpp::bytes_to_int(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_int_to_bytes(const int n,
                                              unsigned char* bytes,
                                              bool big_endian)
{
    return libcpp::int_to_bytes(n, bytes, big_endian);
}

LIBCPP_API long libcpp_bytes_to_long(const unsigned char* bytes,
                                     bool big_endian)
{
    return libcpp::bytes_to_long(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_long_to_bytes(const long n,
                                               unsigned char* bytes,
                                               bool big_endian)
{
    return libcpp::long_to_bytes(n, bytes, big_endian);
}

LIBCPP_API long long libcpp_bytes_to_long_long(const unsigned char* bytes,
                                               bool big_endian)
{
    return libcpp::bytes_to_long_long(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_long_long_to_bytes(const long long n,
                                                    unsigned char* bytes,
                                                    bool big_endian)
{
    return libcpp::long_long_to_bytes(n, bytes, big_endian);
}

LIBCPP_API short libcpp_bytes_to_short(unsigned char* bytes, bool big_endian)
{
    return libcpp::bytes_to_short(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_short_to_bytes(const short n,
                                                unsigned char* bytes,
                                                bool big_endian)
{
    return libcpp::short_to_bytes(n, bytes, big_endian);
}

LIBCPP_API double libcpp_bytes_to_double(const unsigned char* bytes)
{
    return libcpp::bytes_to_double(bytes);
}

LIBCPP_API unsigned char* libcpp_double_to_bytes(const double num,
                                                 unsigned char* bytes)
{
    return libcpp::double_to_bytes(num, bytes);
}

LIBCPP_API float libcpp_bytes_to_float(const unsigned char* bytes)
{
    return libcpp::bytes_to_float(bytes);
}

LIBCPP_API unsigned char* libcpp_float_to_bytes(const float f,
                                                unsigned char* bytes)
{
    return libcpp::float_to_bytes(f, bytes);
}