
#include <gtest/gtest.h>
#include <hj/encoding/bytes.hpp>
#include <array>
#include <string>
#include <cstring>

TEST(bytes, bool_bytes)
{
    std::array<unsigned char, 1> buf = {0x01};
    ASSERT_TRUE(hj::bytes_to_bool(buf));
    buf[0] = 0x0;
    ASSERT_FALSE(hj::bytes_to_bool(buf));
    buf[0] = 0xFF;
    ASSERT_TRUE(hj::bytes_to_bool(buf));
    buf[0] = 0x2;
    ASSERT_FALSE(hj::bytes_to_bool(buf));

    buf[0] = 0x0;
    hj::bool_to_bytes(true, buf);
    ASSERT_EQ(buf[0], 0x1);
    hj::bool_to_bytes(false, buf);
    ASSERT_EQ(buf[0], 0x0);
}

TEST(bytes, int32_bytes)
{
    std::array<unsigned char, 4> buf = {0x0F, 0x00, 0x00, 0x00};
    ASSERT_EQ(hj::bytes_to_int32(buf, true), 0x0F000000);
    ASSERT_EQ(hj::bytes_to_int32(buf, false), 0x0F);

    int32_t n = 0x12345678;
    hj::int32_to_bytes(n, buf, true);
    ASSERT_EQ(buf[0], 0x12);
    ASSERT_EQ(buf[1], 0x34);
    ASSERT_EQ(buf[2], 0x56);
    ASSERT_EQ(buf[3], 0x78);
    hj::int32_to_bytes(n, buf, false);
    ASSERT_EQ(buf[0], 0x78);
    ASSERT_EQ(buf[1], 0x56);
    ASSERT_EQ(buf[2], 0x34);
    ASSERT_EQ(buf[3], 0x12);
}

TEST(bytes, int64_bytes)
{
    std::array<unsigned char, 8> buf =
        {0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(hj::bytes_to_int64(buf, true), 0x0F00000000000000LL);
    ASSERT_EQ(hj::bytes_to_int64(buf, false), 0x0FLL);

    int64_t n = 0x123456789ABCDEF0LL;
    hj::int64_to_bytes(n, buf, true);
    ASSERT_EQ(buf[0], 0x12);
    ASSERT_EQ(buf[1], 0x34);
    ASSERT_EQ(buf[2], 0x56);
    ASSERT_EQ(buf[3], 0x78);
    ASSERT_EQ(buf[4], 0x9A);
    ASSERT_EQ(buf[5], 0xBC);
    ASSERT_EQ(buf[6], 0xDE);
    ASSERT_EQ(buf[7], 0xF0);
    hj::int64_to_bytes(n, buf, false);
    ASSERT_EQ(buf[0], 0xF0);
    ASSERT_EQ(buf[1], 0xDE);
    ASSERT_EQ(buf[2], 0xBC);
    ASSERT_EQ(buf[3], 0x9A);
    ASSERT_EQ(buf[4], 0x78);
    ASSERT_EQ(buf[5], 0x56);
    ASSERT_EQ(buf[6], 0x34);
    ASSERT_EQ(buf[7], 0x12);
}

TEST(bytes, float_bytes)
{
    std::array<unsigned char, 4> buf = {0};
    float                        f   = 3.1415926f;
    hj::float_to_bytes(f, buf);
    float f2 = hj::bytes_to_float(buf);
    ASSERT_NEAR(f, f2, 1e-6f);
}

TEST(bytes, double_bytes)
{
    std::array<unsigned char, 8> buf = {0};
    double                       d   = 3.141592653589793;
    hj::double_to_bytes(d, buf);
    double d2 = hj::bytes_to_double(buf);
    ASSERT_NEAR(d, d2, 1e-12);
}

TEST(bytes, string_bytes)
{
    std::string                   s   = "hello world!";
    std::array<unsigned char, 32> buf = {0};
    hj::string_to_bytes(s, buf);
    std::string s2 = hj::bytes_to_string(buf, s.size());
    ASSERT_EQ(s, s2);
    std::string s3(40, 'A');
    hj::string_to_bytes(s3, buf);
    std::string s4 = hj::bytes_to_string(buf, buf.size());
    ASSERT_EQ(s4.size(), buf.size());
    ASSERT_EQ(std::count(s4.begin(), s4.end(), 'A'), (int) buf.size());
}

TEST(bytes, edge_cases)
{
    std::array<unsigned char, 4> buf4 = {0};
    hj::int32_to_bytes(INT32_MAX, buf4);
    ASSERT_EQ(hj::bytes_to_int32(buf4), INT32_MAX);
    hj::int32_to_bytes(INT32_MIN, buf4);
    ASSERT_EQ(hj::bytes_to_int32(buf4), INT32_MIN);
    std::array<unsigned char, 8> buf8 = {0};
    hj::int64_to_bytes(INT64_MAX, buf8);
    ASSERT_EQ(hj::bytes_to_int64(buf8), INT64_MAX);
    hj::int64_to_bytes(INT64_MIN, buf8);
    ASSERT_EQ(hj::bytes_to_int64(buf8), INT64_MIN);
    float f = -0.0f;
    hj::float_to_bytes(f, buf4);
    ASSERT_EQ(std::signbit(hj::bytes_to_float(buf4)), true);
    double d = -0.0;
    hj::double_to_bytes(d, buf8);
    ASSERT_EQ(std::signbit(hj::bytes_to_double(buf8)), true);
}