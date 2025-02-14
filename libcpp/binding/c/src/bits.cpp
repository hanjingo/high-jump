#include <libcpp/encoding/bits.hpp>

extern "C" {
#include "bits.h"
}

LIBCPP_API bool libcpp_bits_get(const int src, const unsigned int pos)
{
    return libcpp::Bits::Get(src, pos);
}

LIBCPP_API int libcpp_bits_put(int src, const unsigned int pos, const bool bit)
{
    return libcpp::Bits::Put(src, pos, bit);
}

LIBCPP_API int libcpp_bits_reset(int src, const bool bit)
{
    return libcpp::Bits::Reset(src, bit);
}


LIBCPP_API int libcpp_bits_flip(int src)
{
    return libcpp::Bits::Flip(src);
}


LIBCPP_API unsigned int libcpp_bits_to_string(const int src, char* buf)
{
    auto str = libcpp::Bits::ToString(src);
    unsigned int sz = str.length();
    memcpy(buf, str.c_str(), sz);
    return sz;
}