#include <libcpp/encoding/endian.hpp>

extern "C" {
#include "endian.h"
}

LIBCPP_API short libcpp_convert_big_endian_short(const short n)
{
    return libcpp::BigEndian::Convert(n);
}

LIBCPP_API int libcpp_convert_big_endian(const int n)
{
    return libcpp::BigEndian::Convert(n);
}

LIBCPP_API long libcpp_convert_big_endian_long(const long n)
{
    return libcpp::BigEndian::Convert(n);
}

LIBCPP_API short libcpp_convert_little_endian_short(const short n)
{
    return libcpp::LittleEndian::Convert(n);
}

LIBCPP_API int libcpp_convert_little_endian(const int n)
{
    return libcpp::LittleEndian::Convert(n);
}

LIBCPP_API long libcpp_convert_little_endian_long(const long n)
{
    return libcpp::LittleEndian::Convert(n);
}