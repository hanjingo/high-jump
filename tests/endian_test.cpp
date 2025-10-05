#include <gtest/gtest.h>
#include <hj/encoding/endian.hpp>

bool is_big_endian()
{
    int n = 1;
    return *(char *) (&n) == 0;
}

TEST(endian, is_big_endian)
{
    ASSERT_EQ(hj::is_big_endian(), is_big_endian());
}

TEST(endian, big_endian_convert)
{
    unsigned int n = 0x1;
    ASSERT_EQ(hj::big_endian::convert(n), 0x01000000);
}

TEST(endian, little_endian_convert)
{
    unsigned int n = 0x1;
    ASSERT_EQ(hj::little_endian::convert(n), 0x01000000);
}