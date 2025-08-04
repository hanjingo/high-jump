#include <gtest/gtest.h>
#include <libcpp/types/endian.hpp>

bool is_big_endian()
{
    int n = 1;
    return *(char*)(&n) == 1;
}

TEST(endian, is_big_endian)
{
    ASSERT_EQ(libcpp::is_big_endian(), is_big_endian());
}

TEST(endian, big_endian_covert)
{
    unsigned int n = 0x1;
    ASSERT_EQ(libcpp::big_endian::covert(n), 0x01000000);
}

TEST(endian, little_endian_covert)
{
    unsigned int n = 0x1;
    ASSERT_EQ(libcpp::little_endian::covert(n), 0x01000000);
}