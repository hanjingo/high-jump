#include <gtest/gtest.h>
#include <hj/encoding/bytes.hpp>
#include <array>
#include <string>
#include <cstring>
#include <cmath>

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
    hj::bool_to_bytes(buf, true);
    ASSERT_EQ(buf[0], 0x1);
    hj::bool_to_bytes(buf, false);
    ASSERT_EQ(buf[0], 0x0);
}

TEST(bytes, int32_bytes)
{
    std::array<unsigned char, 4> buf = {0x0F, 0x00, 0x00, 0x00};
    ASSERT_EQ(hj::bytes_to_int32(buf, true), 0x0F000000);
    ASSERT_EQ(hj::bytes_to_int32(buf, false), 0x0F);

    int32_t n = 0x12345678;
    hj::int32_to_bytes(buf, n, true);
    ASSERT_EQ(buf[0], 0x12);
    ASSERT_EQ(buf[1], 0x34);
    ASSERT_EQ(buf[2], 0x56);
    ASSERT_EQ(buf[3], 0x78);
    hj::int32_to_bytes(buf, n, false);
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
    hj::int64_to_bytes(buf, n, true);
    ASSERT_EQ(buf[0], 0x12);
    ASSERT_EQ(buf[1], 0x34);
    ASSERT_EQ(buf[2], 0x56);
    ASSERT_EQ(buf[3], 0x78);
    ASSERT_EQ(buf[4], 0x9A);
    ASSERT_EQ(buf[5], 0xBC);
    ASSERT_EQ(buf[6], 0xDE);
    ASSERT_EQ(buf[7], 0xF0);
    hj::int64_to_bytes(buf, n, false);
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
    hj::float_to_bytes(buf, f);
    float f2 = hj::bytes_to_float(buf);
    ASSERT_NEAR(f, f2, 1e-6f);
}

TEST(bytes, double_bytes)
{
    std::array<unsigned char, 8> buf = {0};
    double                       d   = 3.141592653589793;
    hj::double_to_bytes(buf, d);
    double d2 = hj::bytes_to_double(buf);
    ASSERT_NEAR(d, d2, 1e-12);
}

TEST(bytes, string_bytes)
{
    std::string                   s   = "hello world!";
    std::array<unsigned char, 32> buf = {0};
    hj::string_to_bytes(buf, s);
    std::string s2 = hj::bytes_to_string(buf, s.size());
    ASSERT_EQ(s, s2);
    std::string s3(40, 'A');
    hj::string_to_bytes(buf, s3);
    std::string s4 = hj::bytes_to_string(buf, buf.size());
    ASSERT_EQ(s4.size(), buf.size());
    ASSERT_EQ(std::count(s4.begin(), s4.end(), 'A'), (int) buf.size());
}

TEST(bytes, edge_cases)
{
    std::array<unsigned char, 4> buf4 = {0};
    hj::int32_to_bytes(buf4, INT32_MAX);
    ASSERT_EQ(hj::bytes_to_int32(buf4), INT32_MAX);
    hj::int32_to_bytes(buf4, INT32_MIN);
    ASSERT_EQ(hj::bytes_to_int32(buf4), INT32_MIN);
    std::array<unsigned char, 8> buf8 = {0};
    hj::int64_to_bytes(buf8, INT64_MAX);
    ASSERT_EQ(hj::bytes_to_int64(buf8), INT64_MAX);
    hj::int64_to_bytes(buf8, INT64_MIN);
    ASSERT_EQ(hj::bytes_to_int64(buf8), INT64_MIN);
    float f = -0.0f;
    hj::float_to_bytes(buf4, f);
    ASSERT_EQ(std::signbit(hj::bytes_to_float(buf4)), true);
    double d = -0.0;
    hj::double_to_bytes(buf8, d);
    ASSERT_EQ(std::signbit(hj::bytes_to_double(buf8)), true);
}

TEST(bytes, pointer_overloads)
{
    // bool pointer
    unsigned char bbuf[1] = {0};
    size_t        bsz     = 1;
    hj::bool_to_bytes(bbuf, bsz, true);
    ASSERT_EQ(bsz, 1u);
    ASSERT_EQ(bbuf[0], 0x1);
    ASSERT_TRUE(hj::bytes_to_bool(bbuf, bsz));

    hj::bool_to_bytes(bbuf, bsz, false);
    ASSERT_FALSE(hj::bytes_to_bool(bbuf, bsz));

    // int32 pointer
    unsigned char ibuf[4] = {0};
    size_t        isz     = 4;
    int32_t       in      = 0x12345678;
    hj::int32_to_bytes(ibuf, isz, in, true);
    ASSERT_EQ(isz, 4u);
    ASSERT_EQ(hj::bytes_to_int32(ibuf, isz, true), in);
    hj::int32_to_bytes(ibuf, isz, in, false);
    ASSERT_EQ(hj::bytes_to_int32(ibuf, isz, false), in);

    // int64 pointer
    unsigned char lbuf[8] = {0};
    size_t        lsz     = 8;
    int64_t       ln      = 0x123456789ABCDEF0LL;
    hj::int64_to_bytes(lbuf, lsz, ln, true);
    ASSERT_EQ(lsz, 8u);
    ASSERT_EQ(hj::bytes_to_int64(lbuf, lsz, true), ln);
    hj::int64_to_bytes(lbuf, lsz, ln, false);
    ASSERT_EQ(hj::bytes_to_int64(lbuf, lsz, false), ln);

    // float pointer
    unsigned char fbuf[4] = {0};
    size_t        fsz     = 4;
    float         f       = 3.1415926f;
    hj::float_to_bytes(fbuf, fsz, f);
    ASSERT_EQ(fsz, 4u);
    ASSERT_NEAR(hj::bytes_to_float(fbuf, fsz), f, 1e-6f);

    // double pointer
    unsigned char dbuf[8] = {0};
    size_t        dsz     = 8;
    double        d       = 2.718281828459045;
    hj::double_to_bytes(dbuf, dsz, d);
    ASSERT_EQ(dsz, 8u);
    ASSERT_NEAR(hj::bytes_to_double(dbuf, dsz), d, 1e-12);

    // string pointer
    unsigned char sbuf[32] = {0};
    size_t        ssz      = sizeof(sbuf);
    std::string   s        = "pointer test";
    hj::string_to_bytes(sbuf, ssz, s);
    // ssz should be min(original_size, s.size())
    ASSERT_EQ(ssz, s.size());
    std::string s2 = hj::bytes_to_string(sbuf, ssz);
    ASSERT_EQ(s2, s);
}