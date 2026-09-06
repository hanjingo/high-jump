#include <gtest/gtest.h>
#include <hj/encoding/bytes.hpp>

#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

TEST(bytes, bool_bytes)
{
    std::array<unsigned char, 1> buf = {0x01};
    ASSERT_TRUE(hj::bytes_to_bool(buf));
    buf[0] = 0x0;
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

TEST(bytes, float_endianness)
{
    std::array<uint8_t, 4> buf_be = {0};
    std::array<uint8_t, 4> buf_le = {0};
    float                  val    = 123.456f;

    hj::try_float_to_bytes(buf_be, val, true);
    hj::try_float_to_bytes(buf_le, val, false);

    EXPECT_EQ(buf_be[0], buf_le[3]);
    EXPECT_EQ(buf_be[1], buf_le[2]);

    EXPECT_NEAR(*hj::try_bytes_to_float(buf_be, true), val, 1e-5f);
    EXPECT_NEAR(*hj::try_bytes_to_float(buf_le, false), val, 1e-5f);
}

TEST(bytes, safe_vector_access)
{
    std::vector<uint8_t> vec;

    EXPECT_EQ(hj::try_bytes_to_int32(vec), std::nullopt);
    EXPECT_FALSE(hj::try_int32_to_bytes(vec, 42));

    vec.resize(4);
    EXPECT_TRUE(hj::try_int32_to_bytes(vec, 0x12345678));
    EXPECT_EQ(*hj::try_bytes_to_int32(vec), 0x12345678);
}

TEST(bytes, int32_extreme_values)
{
    const int32_t test_cases[] = {0,
                                  1,
                                  -1,
                                  std::numeric_limits<int32_t>::min(),
                                  std::numeric_limits<int32_t>::max(),
                                  static_cast<int32_t>(0x55555555),
                                  static_cast<int32_t>(0xAAAAAAAA)};

    for(int32_t val : test_cases)
    {
        std::array<uint8_t, 4> buf_be{};
        std::array<uint8_t, 4> buf_le{};

        // Safe API BE & LE
        EXPECT_TRUE(hj::try_int32_to_bytes(buf_be, val, true));
        EXPECT_TRUE(hj::try_int32_to_bytes(buf_le, val, false));

        auto res_be = hj::try_bytes_to_int32(buf_be, true);
        auto res_le = hj::try_bytes_to_int32(buf_le, false);

        ASSERT_TRUE(res_be.has_value());
        ASSERT_TRUE(res_le.has_value());

        EXPECT_EQ(*res_be, val);
        EXPECT_EQ(*res_le, val);
    }
}

TEST(bytes, int64_extreme_values)
{
    const int64_t test_cases[] = {0LL,
                                  1LL,
                                  -1LL,
                                  std::numeric_limits<int64_t>::min(),
                                  std::numeric_limits<int64_t>::max(),
                                  0x5555555555555555LL,
                                  static_cast<int64_t>(0xAAAAAAAAAAAAAAAAULL)};

    for(int64_t val : test_cases)
    {
        std::array<uint8_t, 8> buf_be{};
        std::array<uint8_t, 8> buf_le{};

        EXPECT_TRUE(hj::try_int64_to_bytes(buf_be, val, true));
        EXPECT_TRUE(hj::try_int64_to_bytes(buf_le, val, false));

        auto res_be = hj::try_bytes_to_int64(buf_be, true);
        auto res_le = hj::try_bytes_to_int64(buf_le, false);

        ASSERT_TRUE(res_be.has_value());
        ASSERT_TRUE(res_le.has_value());

        EXPECT_EQ(*res_be, val);
        EXPECT_EQ(*res_le, val);
    }
}

TEST(bytes, endianness_cross_decode)
{
    const uint32_t         val = 0x12345678;
    std::array<uint8_t, 4> buf_be{};
    std::array<uint8_t, 4> buf_le{};

    hj::try_uint32_to_bytes(buf_be, val, true);  // 12 34 56 78
    hj::try_uint32_to_bytes(buf_le, val, false); // 78 56 34 12

    EXPECT_EQ(buf_be[0], buf_le[3]);
    EXPECT_EQ(buf_be[1], buf_le[2]);
    EXPECT_EQ(buf_be[2], buf_le[1]);
    EXPECT_EQ(buf_be[3], buf_le[0]);

    auto decoded_cross = hj::try_bytes_to_uint32(buf_be, false);
    ASSERT_TRUE(decoded_cross.has_value());
    EXPECT_EQ(*decoded_cross, 0x78563412u);
}

TEST(bytes, buffer_boundary_stair_case)
{
    uint8_t raw_buf[10] = {0x12, 0x34, 0x56, 0x78, 0x9A};

    // size = 0
    EXPECT_EQ(hj::try_bytes_to_int32(hj::byte_view(raw_buf, 0)), std::nullopt);
    EXPECT_FALSE(
        hj::try_int32_to_bytes(hj::mutable_byte_view(raw_buf, 0), 100));

    // size = 1
    EXPECT_EQ(hj::try_bytes_to_int32(hj::byte_view(raw_buf, 1)), std::nullopt);
    EXPECT_FALSE(
        hj::try_int32_to_bytes(hj::mutable_byte_view(raw_buf, 1), 100));

    // size = 3
    EXPECT_EQ(hj::try_bytes_to_int32(hj::byte_view(raw_buf, 3)), std::nullopt);
    EXPECT_FALSE(
        hj::try_int32_to_bytes(hj::mutable_byte_view(raw_buf, 3), 100));

    // size = 4
    auto res4 = hj::try_bytes_to_int32(hj::byte_view(raw_buf, 4), true);
    ASSERT_TRUE(res4.has_value());
    EXPECT_EQ(*res4, 0x12345678);
    EXPECT_TRUE(hj::try_int32_to_bytes(hj::mutable_byte_view(raw_buf, 4), 100));

    // size = 5
    auto res5 = hj::try_bytes_to_int32(hj::byte_view(raw_buf, 5), true);
    ASSERT_TRUE(res5.has_value());
    EXPECT_TRUE(hj::try_int32_to_bytes(hj::mutable_byte_view(raw_buf, 5), 100));
}

TEST(bytes, pointer_overload_release_safety)
{
    uint8_t     buf[10] = {0};
    std::size_t sz      = 2;

    unsigned char *ptr = hj::int32_to_bytes(buf, sz, 0x12345678, true);
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(sz, 2u);

    sz  = 0;
    ptr = hj::int32_to_bytes(buf, sz, 0x12345678, true);
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(sz, 0u);

    sz  = 4;
    ptr = hj::int32_to_bytes(buf, sz, 0x12345678, true);
    EXPECT_EQ(ptr, buf);
    EXPECT_EQ(sz, 4u);
    EXPECT_EQ(buf[0], 0x12);
    EXPECT_EQ(buf[3], 0x78);
}

TEST(bytes, string_api_semantics_comparison)
{
    std::array<uint8_t, 5> small_buf{};
    std::string            long_str = "Hello World"; // 11 char

    bool try_ok =
        hj::try_string_to_bytes(hj::mutable_byte_view(small_buf), long_str);
    EXPECT_FALSE(try_ok);

    std::size_t copied =
        hj::string_to_bytes(hj::mutable_byte_view(small_buf), long_str);
    EXPECT_EQ(copied, 5u);
    EXPECT_EQ(std::string(reinterpret_cast<char *>(small_buf.data()), 5),
              "Hello");
}

TEST(bytes, roundtrip_property_test)
{
    std::mt19937_64                        rng(1337);
    std::uniform_int_distribution<int32_t> dist_i32(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
    std::uniform_int_distribution<int64_t> dist_i64(
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max());
    std::uniform_real_distribution<float>  dist_f32(-1e6f, 1e6f);
    std::uniform_real_distribution<double> dist_f64(-1e12, 1e12);

    constexpr int kIterations = 10000;

    for(int i = 0; i < kIterations; ++i)
    {
        // 1. int32 Round-trip (BE & LE)
        {
            int32_t                val = dist_i32(rng);
            std::array<uint8_t, 4> buf_be{}, buf_le{};

            ASSERT_TRUE(hj::try_int32_to_bytes(buf_be, val, true));
            ASSERT_TRUE(hj::try_int32_to_bytes(buf_le, val, false));

            EXPECT_EQ(hj::try_bytes_to_int32(buf_be, true), val);
            EXPECT_EQ(hj::try_bytes_to_int32(buf_le, false), val);
        }

        // 2. int64 Round-trip (BE & LE)
        {
            int64_t                val = dist_i64(rng);
            std::array<uint8_t, 8> buf_be{}, buf_le{};

            ASSERT_TRUE(hj::try_int64_to_bytes(buf_be, val, true));
            ASSERT_TRUE(hj::try_int64_to_bytes(buf_le, val, false));

            EXPECT_EQ(hj::try_bytes_to_int64(buf_be, true), val);
            EXPECT_EQ(hj::try_bytes_to_int64(buf_le, false), val);
        }

        // 3. float Round-trip (BE & LE)
        {
            float                  val = dist_f32(rng);
            std::array<uint8_t, 4> buf_be{}, buf_le{};

            ASSERT_TRUE(hj::try_float_to_bytes(buf_be, val, true));
            ASSERT_TRUE(hj::try_float_to_bytes(buf_le, val, false));

            EXPECT_FLOAT_EQ(*hj::try_bytes_to_float(buf_be, true), val);
            EXPECT_FLOAT_EQ(*hj::try_bytes_to_float(buf_le, false), val);
        }

        // 4. double Round-trip (BE & LE)
        {
            double                 val = dist_f64(rng);
            std::array<uint8_t, 8> buf_be{}, buf_le{};

            ASSERT_TRUE(hj::try_double_to_bytes(buf_be, val, true));
            ASSERT_TRUE(hj::try_double_to_bytes(buf_le, val, false));

            EXPECT_DOUBLE_EQ(*hj::try_bytes_to_double(buf_be, true), val);
            EXPECT_DOUBLE_EQ(*hj::try_bytes_to_double(buf_le, false), val);
        }
    }
}

TEST(bytes, ieee754_special_floats_and_nan_payload)
{
    // 1. Infinity
    {
        float pos_inf = std::numeric_limits<float>::infinity();
        float neg_inf = -std::numeric_limits<float>::infinity();

        std::array<uint8_t, 4> buf_pos{}, buf_neg{};
        hj::try_float_to_bytes(buf_pos, pos_inf);
        hj::try_float_to_bytes(buf_neg, neg_inf);

        float res_pos = *hj::try_bytes_to_float(buf_pos);
        float res_neg = *hj::try_bytes_to_float(buf_neg);

        EXPECT_TRUE(std::isinf(res_pos));
        EXPECT_FALSE(std::signbit(res_pos));

        EXPECT_TRUE(std::isinf(res_neg));
        EXPECT_TRUE(std::signbit(res_neg));
    }

    // 2. Quiet NaN & Signaling NaN (semantic test using std::isnan)
    {
        float quiet_nan     = std::numeric_limits<float>::quiet_NaN();
        float signaling_nan = std::numeric_limits<float>::signaling_NaN();

        std::array<uint8_t, 4> buf_q{}, buf_s{};
        hj::try_float_to_bytes(buf_q, quiet_nan);
        hj::try_float_to_bytes(buf_s, signaling_nan);

        float decoded_q = *hj::try_bytes_to_float(buf_q);
        float decoded_s = *hj::try_bytes_to_float(buf_s);

        EXPECT_TRUE(std::isnan(decoded_q));
        EXPECT_TRUE(std::isnan(decoded_s));
    }

    // 3. NaN Custom Payload Bit-Pattern (binary-level serialization test)
    {
        const uint32_t custom_nan_bits = 0x7FC01234u;
        float          custom_nan;
        std::memcpy(&custom_nan, &custom_nan_bits, sizeof(custom_nan));

        std::array<uint8_t, 4> buf_be{}, buf_le{};
        hj::try_float_to_bytes(buf_be, custom_nan, true);
        hj::try_float_to_bytes(buf_le, custom_nan, false);

        float decoded_be = *hj::try_bytes_to_float(buf_be, true);
        float decoded_le = *hj::try_bytes_to_float(buf_le, false);

        EXPECT_TRUE(std::isnan(decoded_be));
        EXPECT_TRUE(std::isnan(decoded_le));

        uint32_t decoded_bits_be = 0, decoded_bits_le = 0;
        std::memcpy(&decoded_bits_be, &decoded_be, sizeof(decoded_bits_be));
        std::memcpy(&decoded_bits_le, &decoded_le, sizeof(decoded_bits_le));

        EXPECT_EQ(decoded_bits_be, custom_nan_bits);
        EXPECT_EQ(decoded_bits_le, custom_nan_bits);
    }
}

TEST(bytes, modern_explicit_api_test)
{
    std::array<uint8_t, 8> buf{};

    // 1. BE Write & Read
    EXPECT_TRUE(hj::bytes::write_be_i32(buf, 0x12345678));
    EXPECT_EQ(*hj::bytes::read_be_i32(buf), 0x12345678);
    EXPECT_EQ(buf[0], 0x12);
    EXPECT_EQ(buf[3], 0x78);

    // 2. LE Write & Read
    EXPECT_TRUE(hj::bytes::write_le_u64(buf, 0x1122334455667788ULL));
    EXPECT_EQ(*hj::bytes::read_le_u64(buf), 0x1122334455667788ULL);
    EXPECT_EQ(buf[0], 0x88);
    EXPECT_EQ(buf[7], 0x11);

    // 3. Float & Double
    EXPECT_TRUE(hj::bytes::write_be_f32(buf, 3.14159f));
    EXPECT_FLOAT_EQ(*hj::bytes::read_be_f32(buf), 3.14159f);

    EXPECT_TRUE(hj::bytes::write_le_f64(buf, 2.718281828459));
    EXPECT_DOUBLE_EQ(*hj::bytes::read_le_f64(buf), 2.718281828459);
}