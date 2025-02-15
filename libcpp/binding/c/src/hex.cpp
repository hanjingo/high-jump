#include <libcpp/encoding/hex.hpp>

extern "C" {
#include "hex.h"
}

LIBCPP_API int libcpp_from_hex_str(const char* str)
{
    return libcpp::hex::from_str<int>(str);
}

LIBCPP_API char* libcpp_to_hex_str(const int n, char* buf)
{
    return libcpp::hex::to_str(n, buf);
}