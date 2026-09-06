#include <gtest/gtest.h>
#include <hj/encoding/bits.hpp>
#include <climits>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <type_traits>

template <typename T, typename = void>
struct is_get_valid : std::false_type
{
};

template <typename T>
struct is_get_valid<
    T,
    std::void_t<decltype(hj::bits::get(std::declval<T>(), std::size_t{}))>>
    : std::true_type
{
};

template <typename T>
inline constexpr bool is_get_valid_v = is_get_valid<T>::value;

static_assert(!is_get_valid_v<int8_t>, "hj::bits::get must reject int8_t");
static_assert(!is_get_valid_v<int16_t>, "hj::bits::get must reject int16_t");
static_assert(!is_get_valid_v<int32_t>, "hj::bits::get must reject int32_t");
static_assert(!is_get_valid_v<int64_t>, "hj::bits::get must reject int64_t");
static_assert(!is_get_valid_v<bool>, "hj::bits::get must reject bool");

static_assert(is_get_valid_v<uint8_t>, "hj::bits::get must accept uint8_t");
static_assert(is_get_valid_v<uint16_t>, "hj::bits::get must accept uint16_t");
static_assert(is_get_valid_v<uint32_t>, "hj::bits::get must accept uint32_t");
static_assert(is_get_valid_v<uint64_t>, "hj::bits::get must accept uint64_t");

TEST(bits, size_max_overflow_protection)
{
    constexpr std::size_t size_max = std::numeric_limits<std::size_t>::max();
    uint32_t              val      = 0;

    EXPECT_FALSE(hj::bits::try_extract(uint32_t{0}, size_max, 1, val));
    EXPECT_THROW((void) hj::bits::extract(uint32_t{0}, size_max, 1),
                 std::out_of_range);

    EXPECT_FALSE(hj::bits::try_extract(uint32_t{0}, size_max - 1, 4, val));
    EXPECT_THROW((void) hj::bits::extract(uint32_t{0}, size_max - 1, 4),
                 std::out_of_range);

    uint32_t src = 0;
    EXPECT_FALSE(hj::bits::try_insert(src, size_max, 1, uint32_t{1}));
    EXPECT_FALSE(hj::bits::try_insert(src, size_max - 1, 4, uint32_t{1}));
}

TEST(bits, width_boundary)
{
    using T                 = uint32_t;
    constexpr std::size_t N = sizeof(T) * CHAR_BIT; // 32
    T                     x = 0xABCD1234U;

    EXPECT_EQ(hj::bits::extract(x, N - 1, 1), 1U);

    EXPECT_EQ(hj::bits::extract(x, 0, N - 1), 0x2BCD1234U);

    EXPECT_EQ(hj::bits::extract(x, 1, N - 1), (0xABCD1234U >> 1));

    EXPECT_EQ(hj::bits::extract(x, 0, N), 0xABCD1234U);

    EXPECT_THROW((void) hj::bits::extract(x, 0, N + 1), std::out_of_range);
}

TEST(bits, property_based_invariants)
{
    std::mt19937_64                         rng(1337);
    std::uniform_int_distribution<uint64_t> dist_u64;

    for(int i = 0; i < 1000; ++i)
    {
        uint64_t    x   = dist_u64(rng);
        std::size_t pos = dist_u64(rng) % 64;

        // Invariant 1: put(x, pos, true) -> get(x, pos) == true
        uint64_t x1 = x;
        hj::bits::put(x1, pos, true);
        EXPECT_TRUE(hj::bits::get(x1, pos));

        // Invariant 2: put(x, pos, false) -> get(x, pos) == false
        uint64_t x2 = x;
        hj::bits::put(x2, pos, false);
        EXPECT_FALSE(hj::bits::get(x2, pos));

        // Invariant 3: extract(insert(x, off, width, v), off, width) == (v & mask)
        std::size_t offset = dist_u64(rng) % 63; // 0..62
        std::size_t width  = 1 + (dist_u64(rng) % (64 - offset));
        uint64_t    v      = dist_u64(rng);

        uint64_t inserted = x;
        hj::bits::insert(inserted, offset, width, v);
        uint64_t extracted = hj::bits::extract(inserted, offset, width);

        uint64_t expected_mask =
            (width == 64) ? ~0ULL : ((1ULL << width) - 1ULL);
        EXPECT_EQ(extracted, v & expected_mask);
    }
}

TEST(bits, extract)
{
    uint32_t data = 0xABCD1234U;
    uint32_t val1 = hj::bits::extract(data, 0, 16);
    ASSERT_EQ(val1, 0x1234U);

    uint32_t val2 = hj::bits::extract(data, 4, 12);
    ASSERT_EQ(val2, 0x123U);

    ASSERT_EQ(hj::bits::extract(data, 0, sizeof(uint32_t) * CHAR_BIT),
              0xABCD1234U);

    EXPECT_THROW((void) hj::bits::extract(data, 16, 20), std::out_of_range);
    EXPECT_THROW((void) hj::bits::extract(data, 0, 0), std::out_of_range);
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
    ASSERT_EQ(data, 0xC3);

    ASSERT_FALSE(hj::bits::try_insert(data, 6, 4, uint8_t(0xF)));
    ASSERT_EQ(data, 0xC3);
}

TEST(bits, get)
{
    ASSERT_TRUE(hj::bits::get(uint32_t(0xFFFFFFFF), 0));
    ASSERT_FALSE(hj::bits::get(uint32_t(0x0), 0));
    ASSERT_TRUE(hj::bits::get(uint8_t(0x80), 7));
    ASSERT_FALSE(hj::bits::get(uint8_t(0x80), 0));

    EXPECT_THROW(
        (void) hj::bits::get(uint8_t(0x80), sizeof(uint8_t) * CHAR_BIT),
        std::out_of_range);
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
    ASSERT_FALSE(hj::bits::try_get(src, sizeof(uint8_t) * CHAR_BIT, val));
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

    EXPECT_THROW((void) hj::bits::put(n, sizeof(uint32_t) * CHAR_BIT),
                 std::out_of_range);
}

TEST(bits, try_put)
{
    uint8_t n = 0;
    ASSERT_TRUE(hj::bits::try_put(n, 0, true));
    ASSERT_EQ(n, 0x1);

    ASSERT_FALSE(hj::bits::try_put(n, sizeof(uint8_t) * CHAR_BIT, true));
    ASSERT_EQ(n, 0x1);
}

TEST(bits, flip_single_bit)
{
    uint8_t n = 0x01;

    hj::bits::flip(n, 0);
    ASSERT_EQ(n, 0x00);

    hj::bits::flip(n, 7);
    ASSERT_EQ(n, 0x80);

    EXPECT_THROW((void) hj::bits::flip(n, sizeof(uint8_t) * CHAR_BIT),
                 std::out_of_range);
}

TEST(bits, try_flip_single_bit)
{
    uint8_t n = 0x00;

    ASSERT_TRUE(hj::bits::try_flip(n, 1));
    ASSERT_EQ(n, 0x02);

    ASSERT_FALSE(hj::bits::try_flip(n, sizeof(uint8_t) * CHAR_BIT));
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
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(0)), sizeof(uint8_t) * CHAR_BIT);
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(1)),
              sizeof(uint8_t) * CHAR_BIT - 1);
    ASSERT_EQ(hj::bits::countl_zero(uint8_t(0x80)), 0);
    ASSERT_EQ(hj::bits::countl_zero(uint32_t(0x00F00000)), 8);
    ASSERT_EQ(hj::bits::countl_zero(uint64_t(0x0000000000000010ULL)), 59);
}

template <typename T>
class SystematicBitPositionTest : public ::testing::Test
{
};

using UnsignedTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
TYPED_TEST_SUITE(SystematicBitPositionTest, UnsignedTypes);

TYPED_TEST(SystematicBitPositionTest, SystematicPositions)
{
    using T                         = TypeParam;
    constexpr std::size_t N         = sizeof(T) * CHAR_BIT;
    constexpr std::size_t pos_first = 0;
    constexpr std::size_t pos_mid   = N / 2;
    constexpr std::size_t pos_last  = N - 1;

    T val = 0;

    hj::bits::put(val, pos_first, true);
    EXPECT_TRUE(hj::bits::get(val, pos_first));
    hj::bits::flip(val, pos_first);
    EXPECT_FALSE(hj::bits::get(val, pos_first));

    hj::bits::put(val, pos_mid, true);
    EXPECT_TRUE(hj::bits::get(val, pos_mid));
    hj::bits::flip(val, pos_mid);
    EXPECT_FALSE(hj::bits::get(val, pos_mid));

    hj::bits::put(val, pos_last, true);
    EXPECT_TRUE(hj::bits::get(val, pos_last));

    hj::bits::put(val, pos_last, false);
    EXPECT_FALSE(hj::bits::get(val, pos_last));
    EXPECT_EQ(val, T(0));
}

TEST(bits, msb_clear_uint64)
{
    uint64_t x = std::numeric_limits<uint64_t>::max();
    hj::bits::put(x, 63, false);
    EXPECT_EQ(x, 0x7FFFFFFFFFFFFFFFULL);
    EXPECT_FALSE(hj::bits::get(x, 63));
}

TEST(bits, buffer_boundary)
{
    uint16_t              val = 0x1234;
    constexpr std::size_t N   = sizeof(val) * CHAR_BIT;
    char                  buf[32];

    EXPECT_FALSE(hj::bits::to_string(val, static_cast<char *>(nullptr), N + 1));
    EXPECT_FALSE(hj::bits::to_string(val, buf, 0));
    EXPECT_FALSE(hj::bits::to_string(val, buf, N));
    EXPECT_TRUE(hj::bits::to_string(val, buf, N + 1));
    EXPECT_STREQ(buf, "0001001000110100");
    EXPECT_TRUE(hj::bits::to_string(val, buf, N + 2));
    EXPECT_STREQ(buf, "0001001000110100");
}