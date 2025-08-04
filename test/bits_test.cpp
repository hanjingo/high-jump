#include <cstdint>

#include <gtest/gtest.h>
#include <libcpp/types/bits.hpp>

using libcpp::bits;

TEST(bits, get)
{
    ASSERT_EQ(libcpp::bits::get(int(0xFFFFFFFF), 1), true);
}

TEST(bits, put)
{
    int n = 0x0;
    ASSERT_EQ(libcpp::bits::put(n, 1), 0x1);
    ASSERT_EQ(libcpp::bits::put(n, 1, false), 0x0);
}

TEST(bits, reset)
{
    int n = 0xFFFFFFFF;
    ASSERT_EQ(libcpp::bits::reset(n, false), 0x0);
}

TEST(bits, flip)
{
    unsigned int n = 0xFFFFFFFF;
    ASSERT_EQ(libcpp::bits::flip(n) == 0x0, true);

    long long n1 = -1;
    ASSERT_EQ(libcpp::bits::flip(n1), 0LL);
}

TEST(bits, to_string)
{
    std::string result = "1111111111111111";
    unsigned short n = 0xFFFF;
    ASSERT_STREQ(libcpp::bits::to_string(n).c_str(), result.c_str());

    char buf[sizeof(unsigned short) * 8 + 1];
    libcpp::bits::to_string(n, buf);
    ASSERT_STREQ(buf, result.c_str());
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