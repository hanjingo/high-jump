#include <libcpp/encoding/endian.hpp>

extern "C"
{
#include "endian.h"
}

LIBCPP_API short libcpp_convert_big_endian_short(const short n)
{
    return libcpp::big_endian::covert(n);
}

LIBCPP_API int libcpp_convert_big_endian(const int n)
{
    return libcpp::big_endian::covert(n);
}

LIBCPP_API short libcpp_convert_little_endian_short(const short n)
{
    return libcpp::little_endian::covert(n);
}

LIBCPP_API int libcpp_convert_little_endian(const int n)
{
    return libcpp::little_endian::covert(n);
}