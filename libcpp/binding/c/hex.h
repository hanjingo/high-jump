#ifndef HEX_H
#define HEX_H

#include <libcpp/binding/c/api.h>

LIBCPP_API int libcpp_from_hex_str(const char* str);

LIBCPP_API char* libcpp_to_hex_str(const int n, char* buf);

#endif