#include <libcpp/encoding/bytes.hpp>

extern "C" {
#include "bytes.h"
}

LIBCPP_API bool libcpp_bytes_to_bool(const unsigned char* bytes)
{
    return libcpp::BytesToBool(bytes);
}

LIBCPP_API unsigned char* libcpp_bool_to_bytes(const bool b, unsigned char* bytes)
{
    return libcpp::BoolToBytes(b, bytes);
}

LIBCPP_API int libcpp_bytes_to_int(const unsigned char* bytes, bool big_endian)
{
    return libcpp::BytesToInt(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_int_to_bytes(const int n, unsigned char* bytes, bool big_endian)
{
    return libcpp::IntToBytes(n, bytes, big_endian);
}

LIBCPP_API long libcpp_bytes_to_long(const unsigned char* bytes, bool big_endian)
{
    return libcpp::BytesToLong(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_long_to_bytes(const long n, unsigned char* bytes, bool big_endian)
{
    return libcpp::LongToBytes(n, bytes, big_endian);
}

LIBCPP_API long long libcpp_bytes_to_long_long(const unsigned char* bytes, bool big_endian)
{
    return libcpp::BytesToLongLong(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_long_long_to_bytes(const long long n, unsigned char* bytes, bool big_endian)
{
    return libcpp::LongLongToBytes(n, bytes, big_endian);
}

LIBCPP_API short libcpp_bytes_to_short(unsigned char* bytes, bool big_endian)
{
    return libcpp::BytesToShort(bytes, big_endian);
}

LIBCPP_API unsigned char* libcpp_short_to_bytes(const short n, unsigned char* bytes, bool big_endian)
{
    return libcpp::ShortToBytes(n, bytes, big_endian);
}

LIBCPP_API double libcpp_bytes_to_double(const unsigned char* bytes)
{
    return libcpp::BytesToDouble(bytes);
}

LIBCPP_API unsigned char* libcpp_double_to_bytes(const double num, unsigned char* bytes)
{
    return libcpp::DoubleToBytes(num, bytes);
}

LIBCPP_API float libcpp_bytes_to_float(const unsigned char* bytes)
{
    return libcpp::BytesToFloat(bytes);
}

LIBCPP_API unsigned char* libcpp_float_to_bytes(const float f, unsigned char* bytes)
{
    return libcpp::FloatToBytes(f, bytes);
}