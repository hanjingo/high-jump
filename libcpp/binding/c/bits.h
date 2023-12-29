#ifndef BIT_H
#define BIT_H

#include <stdbool.h>

#include <libcpp/binding/c/api.h>

LIBCPP_API bool libcpp_bits_get(const int src, const unsigned int pos);

LIBCPP_API int libcpp_bits_put(int src, const unsigned int pos, const bool bit);

LIBCPP_API int libcpp_bits_reset(int src, const bool bit);

LIBCPP_API int libcpp_bits_flip(int src);

LIBCPP_API unsigned int libcpp_bits_to_string(const int src, char* buf);

#endif