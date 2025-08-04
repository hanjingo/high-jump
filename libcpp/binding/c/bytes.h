#ifndef BYTES_H
#define BYTES_H

#include <stdbool.h>

#include <libcpp/binding/c/api.h>

LIBCPP_API bool libcpp_bytes_to_bool(const unsigned char* bytes);

LIBCPP_API unsigned char* libcpp_bool_to_bytes(const bool b,
                                               unsigned char* bytes);

LIBCPP_API int libcpp_bytes_to_int(const unsigned char* bytes, bool big_endian);

LIBCPP_API unsigned char* libcpp_int_to_bytes(const int n,
                                              unsigned char* bytes,
                                              bool big_endian);

LIBCPP_API long libcpp_bytes_to_long(const unsigned char* bytes,
                                     bool big_endian);

LIBCPP_API unsigned char* libcpp_long_to_bytes(const long n,
                                               unsigned char* bytes,
                                               bool big_endian);

LIBCPP_API long long libcpp_bytes_to_long_long(const unsigned char* bytes,
                                               bool big_endian);

LIBCPP_API unsigned char* libcpp_long_long_to_bytes(const long long n,
                                                    unsigned char* bytes,
                                                    bool big_endian);

LIBCPP_API short libcpp_bytes_to_short(unsigned char* bytes, bool big_endian);

LIBCPP_API unsigned char* libcpp_short_to_bytes(const short n,
                                                unsigned char* bytes,
                                                bool big_endian);

LIBCPP_API double libcpp_bytes_to_double(const unsigned char* bytes);

LIBCPP_API unsigned char* libcpp_double_to_bytes(const double num,
                                                 unsigned char* bytes);

LIBCPP_API float libcpp_bytes_to_float(const unsigned char* bytes);

LIBCPP_API unsigned char* libcpp_float_to_bytes(const float f,
                                                unsigned char* bytes);

#endif