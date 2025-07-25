#include <gtest/gtest.h>
#include <libcpp/types/bits.hpp>

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