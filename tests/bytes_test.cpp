#include <gtest/gtest.h>
#include <hj/encoding/bytes.hpp>

TEST(bytes, bytes_to_bool)
{
    unsigned char buf[1] = {0x01};
    ASSERT_EQ(hj::bytes_to_bool(buf), true);

    buf[0] = 0x0;
    ASSERT_EQ(hj::bytes_to_bool(buf), false);

    buf[0] = 0xFF;
    ASSERT_EQ(hj::bytes_to_bool(buf), true);

    buf[0] = 0x2;
    ASSERT_EQ(hj::bytes_to_bool(buf), false);
}

TEST(bytes, bool_to_bytes)
{
    unsigned char buf[1] = {0x0};
    ASSERT_EQ(hj::bool_to_bytes(true, buf)[0], 0x1);
}

TEST(bytes, bytes_to_int)
{
    unsigned char buf[4] = {0x0F, 0x00, 0x00, 0x0};
    ASSERT_EQ(hj::bytes_to_int(buf, true), 0x0F000000);
    ASSERT_EQ(hj::bytes_to_int(buf, false), 0x0F);
}

TEST(bytes, int_to_bytes)
{
    unsigned char buf[4] = {0x0};
    ASSERT_EQ(hj::int_to_bytes(int(0x1), buf, true)[3], 0x1);
    ASSERT_EQ(hj::int_to_bytes(int(0x1), buf, false)[0], 0x1);
}

TEST(bytes, bytes_to_long)
{
    unsigned char buf[4] = {0x0F, 0x00, 0x00, 0x00};
    ASSERT_EQ(hj::bytes_to_long(buf, true), 0x0F000000);
    ASSERT_EQ(hj::bytes_to_long(buf, false), 0x0F);
}

TEST(bytes, long_to_bytes)
{
    unsigned char buf[4] = {0x0};
    ASSERT_EQ(hj::long_to_bytes(int(0x1), buf, true)[3], 0x1);
    ASSERT_EQ(hj::long_to_bytes(int(0x1), buf, false)[0], 0x1);
}

TEST(bytes, bytes_to_long_long)
{
    unsigned char buf[8] = {0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(hj::bytes_to_long_long(buf, true), 0x0F00000000000000);
    ASSERT_EQ(hj::bytes_to_long_long(buf, false), 0x0F);
}

TEST(bytes, long_long_to_bytes)
{
    unsigned char buf[8] = {0x0};
    ASSERT_EQ(hj::long_long_to_bytes(int(0x1), buf, true)[7], 0x1);
    ASSERT_EQ(hj::long_long_to_bytes(int(0x1), buf, false)[0], 0x1);
}

TEST(bytes, bytes_to_short)
{
    unsigned char buf[2] = {0x0F, 0x00};
    ASSERT_EQ(hj::bytes_to_short(buf, true), 0x0F00);
    ASSERT_EQ(hj::bytes_to_short(buf, false), 0x0F);
}

TEST(bytes, short_to_bytes)
{
    unsigned char buf[2] = {0x0};
    ASSERT_EQ(hj::short_to_bytes(int(0x1), buf, true)[1], 0x1);
    ASSERT_EQ(hj::short_to_bytes(int(0x1), buf, false)[0], 0x1);
}

TEST(bytes, bytes_to_double)
{

}

TEST(bytes, double_to_bytes)
{

}

TEST(bytes, bytes_to_float)
{

}

TEST(bytes, float_to_bytes)
{

}

TEST(bytes, bytes_to_string)
{

}

TEST(bytes, string_to_bytes)
{

}