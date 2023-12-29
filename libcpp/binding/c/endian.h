#ifndef ENDIAN_H
#define ENDIAN_H

#include <libcpp/binding/c/api.h>

LIBCPP_API short libcpp_convert_big_endian_short(const short n);
LIBCPP_API int libcpp_convert_big_endian(const int n);
LIBCPP_API long libcpp_convert_big_endian_long(const long n);

LIBCPP_API short libcpp_convert_little_endian_short(const short n);
LIBCPP_API int libcpp_convert_little_endian(const int n);
LIBCPP_API long libcpp_convert_little_endian_long(const long n);

#endif