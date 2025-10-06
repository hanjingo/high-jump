#include <gtest/gtest.h>
#include <hj/encoding/bits.hpp>
#include <cstdint>

using hj::bits;

TEST(bits, get)
{
    ASSERT_TRUE(hj::bits::get(int(0xFFFFFFFF), 1));
    ASSERT_FALSE(hj::bits::get(int(0x0), 1));
    ASSERT_TRUE(hj::bits::get(uint8_t(0x80), 8));
    ASSERT_FALSE(hj::bits::get(uint8_t(0x80), 1));
}

TEST(bits, put)
{
    int n = 0x0;
    hj::bits::put(n, 1);
    ASSERT_EQ(n, 0x1);
    hj::bits::put(n, 2, true);
    ASSERT_EQ(n, 0x3);
    hj::bits::put(n, 2, false);
    ASSERT_EQ(n, 0x1);
    hj::bits::put(n, 32, true);
    ASSERT_EQ(n, 0x80000001);
}


TEST(bits, reset)
{
    int n = 0xFFFFFFFF;
    hj::bits::reset(n, false);
    ASSERT_EQ(n, 0x0);
    hj::bits::reset(n, true);
    ASSERT_EQ(n, ~0);
}

TEST(bits, flip)
{
    unsigned int n = 0xFFFFFFFF;
    hj::bits::flip(n);
    ASSERT_EQ(n, 0x0u);

    long long n1 = -1;
    hj::bits::flip(n1);
    ASSERT_EQ(n1, 0LL);

    uint8_t n2 = 0xAA;
    hj::bits::flip(n2);
    ASSERT_EQ(n2, uint8_t(0x55));
}


TEST(bits, to_string)
{
    std::string    result = "1111111111111111";
    unsigned short n      = 0xFFFF;
    ASSERT_EQ(hj::bits::to_string(n), result);

    char buf[sizeof(unsigned short) * 8 + 1];
    hj::bits::to_string(n, buf);
    ASSERT_STREQ(buf, result.c_str());

    unsigned char n2       = 0;
    std::string   zero_str = hj::bits::to_string(n2);
    ASSERT_EQ(zero_str, std::string("00000000"));
}

TEST(bits, edge_cases)
{
    // get/put/flip/reset on uint8_t
    uint8_t n = 0;
    hj::bits::put(n, 8, true);
    ASSERT_EQ(n, 0x80);
    hj::bits::flip(n);
    ASSERT_EQ(n, 0x7F);
    hj::bits::reset(n, false);
    ASSERT_EQ(n, 0x0);
    hj::bits::reset(n, true);
    ASSERT_EQ(n, 0xFF);
    ASSERT_FALSE(hj::bits::get(n, 9));
}

TEST(bits, count_leading_zeros_all)
{
    ASSERT_EQ(hj::bits::count_leading_zeros(uint8_t(0)), 8);
    ASSERT_EQ(hj::bits::count_leading_zeros(uint8_t(1)), 7);
    ASSERT_EQ(hj::bits::count_leading_zeros(uint8_t(0x80)), 0);
    ASSERT_EQ(hj::bits::count_leading_zeros(uint16_t(0)), 16);
    ASSERT_EQ(hj::bits::count_leading_zeros(uint16_t(1)), 15);
    ASSERT_EQ(hj::bits::count_leading_zeros(uint16_t(0x8000)), 0);
}

TEST(BitsTest, CountLeadingZeros32)
{
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0), 32);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(1), 31);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0x80000000), 0);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0xF0000000), 0);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0x0F000000), 4);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0x00F00000), 8);
    EXPECT_EQ(bits::count_leading_zeros<uint32_t>(0x00000010), 27);
}

TEST(BitsTest, CountLeadingZeros64)
{
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0), 64);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(1), 63);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0x8000000000000000ULL), 0);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0xF000000000000000ULL), 0);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0x0F00000000000000ULL), 4);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0x00F0000000000000ULL), 8);
    EXPECT_EQ(bits::count_leading_zeros<uint64_t>(0x0000000000000010ULL), 59);
}