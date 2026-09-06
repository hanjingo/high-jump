#include <gtest/gtest.h>
#include <hj/encoding/bits.hpp>
#include <cstdint>
#include <limits>
#include <stdexcept>

TEST(bits, extract)
{
    uint32_t data = 0xABCD1234U;
    uint32_t val1 = hj::bits::extract(data, 0, 16);
    ASSERT_EQ(val1, 0x1234U);

    uint32_t val2 = hj::bits::extract(data, 4, 12);
    ASSERT_EQ(val2, 0x123U);

    ASSERT_EQ(hj::bits::extract(data, 0, 32), 0xABCD1234U);

    EXPECT_THROW((void) hj::bits::extract(data, 16, 20),
                 std::out_of_range); // offset + width = 36
    EXPECT_THROW((void) hj::bits::extract(data, 0, 0),
                 std::out_of_range); // width = 0
}

TEST(bits, try_extract)
{
    uint32_t data = 0xABCD1234U;
    uint32_t val  = 0;

    ASSERT_TRUE(hj::bits::try_extract(data, 0, 16, val));
    ASSERT_EQ(val, 0x1234U);

    ASSERT_FALSE(hj::bits::try_extract(data, 16, 20, val));
    ASSERT_EQ(val, 0x1234U);
}

TEST(bits, insert)
{
    uint32_t data = 0x00000000U;

    hj::bits::insert(data, 8, 12, uint32_t(0x567));
    ASSERT_EQ(data, 0x00056700U);

    uint32_t data2 = 0x0;
    hj::bits::insert(data2, 8, 12, uint32_t(0xFF567));
    ASSERT_EQ(data2, 0x00056700U);

    EXPECT_THROW((void) hj::bits::insert(data, 20, 15, uint32_t(0x1)),
                 std::out_of_range);
}

TEST(bits, try_insert)
{
    uint8_t data = 0xFF;

    ASSERT_TRUE(hj::bits::try_insert(data, 2, 4, uint8_t(0x0)));
    ASSERT_EQ(data, 0xC3); // 11000011 = 0xC3

    ASSERT_FALSE(hj::bits::try_insert(data, 6, 4, uint8_t(0xF)));
    ASSERT_EQ(data, 0xC3);
}

TEST(bits, get)
{
    ASSERT_TRUE(hj::bits::get(uint32_t(0xFFFFFFFF), 0));
    ASSERT_FALSE(hj::bits::get(uint32_t(0x0), 0));
    ASSERT_TRUE(hj::bits::get(uint8_t(0x80), 7));
    ASSERT_FALSE(hj::bits::get(uint8_t(0x80), 0));

    EXPECT_THROW((void) hj::bits::get(uint8_t(0x80), 8), std::out_of_range);
}

TEST(bits, try_get)
{
    bool    val = false;
    uint8_t src = 0x80;

    ASSERT_TRUE(hj::bits::try_get(src, 7, val));
    ASSERT_TRUE(val);

    ASSERT_TRUE(hj::bits::try_get(src, 0, val));
    ASSERT_FALSE(val);

    val = true;
    ASSERT_FALSE(hj::bits::try_get(src, 8, val));
    ASSERT_TRUE(val);
}

TEST(bits, put)
{
    uint32_t n = 0x0;
    hj::bits::put(n, 0);
    ASSERT_EQ(n, 0x1u);

    hj::bits::put(n, 1, true);
    ASSERT_EQ(n, 0x3u);

    hj::bits::put(n, 1, false);
    ASSERT_EQ(n, 0x1u);

    hj::bits::put(n, 31, true);
    ASSERT_EQ(n, 0x80000001U);

    EXPECT_THROW((void) hj::bits::put(n, 32), std::out_of_range);
}

TEST(bits, try_put)
{
    uint8_t n = 0;
    ASSERT_TRUE(hj::bits::try_put(n, 0, true));
    ASSERT_EQ(n, 0x1);

    ASSERT_FALSE(hj::bits::try_put(n, 8, true));
    ASSERT_EQ(n, 0x1);
}

TEST(bits, flip_single_bit)
{
    uint8_t n = 0x01; // 00000001

    hj::bits::flip(n, 0);
    ASSERT_EQ(n, 0x00);

    hj::bits::flip(n, 7);
    ASSERT_EQ(n, 0x80);

    EXPECT_THROW((void) hj::bits::flip(n, 8), std::out_of_range);
}

TEST(bits, try_flip_single_bit)
{
    uint8_t n = 0x00;

    ASSERT_TRUE(hj::bits::try_flip(n, 1));
    ASSERT_EQ(n, 0x02);

    ASSERT_FALSE(hj::bits::try_flip(n, 8));
    ASSERT_EQ(n, 0x02);
}

TEST(bits, flip_all_bits)
{
    uint32_t n = 0xFFFFFFFFu;
    hj::bits::flip(n);
    ASSERT_EQ(n, 0x0u);

    uint64_t n1 = ~0ULL;
    hj::bits::flip(n1);
    ASSERT_EQ(n1, 0ULL);

    uint8_t n2 = 0xAA;
    hj::bits::flip(n2);
    ASSERT_EQ(n2, uint8_t(0x55));
}

TEST(bits, clear_and_set_all)
{
    uint32_t n = 0xFFFFFFFFu;
    hj::bits::clear(n);
    ASSERT_EQ(n, 0x0u);

    hj::bits::set_all(n);
    ASSERT_EQ(n, 0xFFFFFFFFu);
}

TEST(bits, to_string)
{
    std::string result = "1111111111111111";
    uint16_t    n      = 0xFFFF;
    ASSERT_EQ(hj::bits::to_string(n), result);

    char buf[17];
    ASSERT_TRUE(hj::bits::to_string(n, buf, sizeof(buf)));
    ASSERT_STREQ(buf, result.c_str());

    uint8_t     n2       = 0;
    std::string zero_str = hj::bits::to_string(n2);
    ASSERT_EQ(zero_str, std::string("00000000"));
}

TEST(bits, countl_zero)
{
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(0)), 8);
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(1)), 7);
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(0x80)), 0);
    ASSERT_EQ(hj::bits::countl_zero(uint32_t(0x00F00000)), 8);
    ASSERT_EQ(hj::bits::countl_zero(uint64_t(0x0000000000000010ULL)), 59);
}

static_assert(!hj::bits::detail::is_valid_bit_type_v<int8_t>,
              "int8_t must be rejected");
static_assert(!hj::bits::detail::is_valid_bit_type_v<int16_t>,
              "int16_t must be rejected");
static_assert(!hj::bits::detail::is_valid_bit_type_v<int32_t>,
              "int32_t must be rejected");
static_assert(!hj::bits::detail::is_valid_bit_type_v<int64_t>,
              "int64_t must be rejected");
static_assert(!hj::bits::detail::is_valid_bit_type_v<bool>,
              "bool must be rejected");

static_assert(hj::bits::detail::is_valid_bit_type_v<uint8_t>,
              "uint8_t must be accepted");
static_assert(hj::bits::detail::is_valid_bit_type_v<uint16_t>,
              "uint16_t must be accepted");
static_assert(hj::bits::detail::is_valid_bit_type_v<uint32_t>,
              "uint32_t must be accepted");
static_assert(hj::bits::detail::is_valid_bit_type_v<uint64_t>,
              "uint64_t must be accepted");

template <typename T>
class SystematicBitPositionTest : public ::testing::Test
{
};

using UnsignedTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
TYPED_TEST_SUITE(SystematicBitPositionTest, UnsignedTypes);

TYPED_TEST(SystematicBitPositionTest, SystematicPositions)
{
    using T                         = TypeParam;
    constexpr std::size_t N         = sizeof(T) * 8;
    constexpr std::size_t pos_first = 0;
    constexpr std::size_t pos_mid   = N / 2;
    constexpr std::size_t pos_last  = N - 1; // MSB

    T val = 0;

    // --- LSB (pos = 0) ---
    hj::bits::put(val, pos_first, true);
    EXPECT_TRUE(hj::bits::get(val, pos_first));
    hj::bits::flip(val, pos_first);
    EXPECT_FALSE(hj::bits::get(val, pos_first));

    // --- MID (pos = N / 2) ---
    hj::bits::put(val, pos_mid, true);
    EXPECT_TRUE(hj::bits::get(val, pos_mid));
    hj::bits::flip(val, pos_mid);
    EXPECT_FALSE(hj::bits::get(val, pos_mid));

    // --- MSB (pos = N - 1) ---
    hj::bits::put(val, pos_last, true);
    EXPECT_TRUE(hj::bits::get(val, pos_last));

    // put(false)
    hj::bits::put(val, pos_last, false);
    EXPECT_FALSE(hj::bits::get(val, pos_last));
    EXPECT_EQ(val, T(0));
}

TEST(bits, msb_clear_uint64)
{
    uint64_t x = std::numeric_limits<uint64_t>::max(); // 0xFFFFFFFFFFFFFFFF
    hj::bits::put(x, 63, false);
    EXPECT_EQ(x, 0x7FFFFFFFFFFFFFFFULL); // MSB (bit 63) reset 0
    EXPECT_FALSE(hj::bits::get(x, 63));
}

TEST(bits, buffer_boundary)
{
    uint16_t              val = 0x1234; // sizeof(uint16_t) * 8 = 16 (N = 16)
    constexpr std::size_t N   = sizeof(val) * 8;
    char                  buf[32];

    EXPECT_FALSE(hj::bits::to_string(val, static_cast<char *>(nullptr), N + 1));

    EXPECT_FALSE(hj::bits::to_string(val, buf, 0));

    EXPECT_FALSE(hj::bits::to_string(val, buf, N));

    EXPECT_TRUE(hj::bits::to_string(val, buf, N + 1));
    EXPECT_STREQ(buf, "0001001000110100");

    EXPECT_TRUE(hj::bits::to_string(val, buf, N + 2));
    EXPECT_STREQ(buf, "0001001000110100");
}