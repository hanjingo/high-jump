#include <gtest/gtest.h>
#include <hj/algo/regex.hpp>

TEST(regex, match)
{
    ASSERT_FALSE(std::regex_match("", hj::REGEX_IDCARD_CN));
    ASSERT_TRUE(std::regex_match("123456789012345678", hj::REGEX_IDCARD_CN));
    ASSERT_TRUE(std::regex_match("12345678901234567x", hj::REGEX_IDCARD_CN));
    ASSERT_TRUE(std::regex_match("12345678901234567X", hj::REGEX_IDCARD_CN));
    ASSERT_FALSE(std::regex_match("12345678901234567a", hj::REGEX_IDCARD_CN));
    ASSERT_FALSE(std::regex_match("1234567890123456789", hj::REGEX_IDCARD_CN));
    ASSERT_FALSE(
        std::regex_match("432503199408097040abc", hj::REGEX_IDCARD_CN));
    ASSERT_FALSE(std::regex_match("43250319940809704", hj::REGEX_IDCARD_CN));
    ASSERT_FALSE(std::regex_match("32503199408097040", hj::REGEX_IDCARD_CN));

    ASSERT_TRUE(std::regex_match("123", hj::REGEX_INTEGER));
    ASSERT_TRUE(std::regex_match("-123", hj::REGEX_INTEGER));
    ASSERT_TRUE(std::regex_match("0", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("0123", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("123.", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("a123", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("123b", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("123.1", hj::REGEX_INTEGER));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_INTEGER));

    ASSERT_TRUE(std::regex_match("123.4", hj::REGEX_DECIMALS1));
    ASSERT_TRUE(std::regex_match("-123.4", hj::REGEX_DECIMALS1));
    ASSERT_TRUE(std::regex_match("123", hj::REGEX_DECIMALS1));
    ASSERT_TRUE(std::regex_match("0.0", hj::REGEX_DECIMALS1));
    ASSERT_FALSE(std::regex_match("123.45", hj::REGEX_DECIMALS1));
    ASSERT_FALSE(std::regex_match("123.a", hj::REGEX_DECIMALS1));
    ASSERT_FALSE(std::regex_match("a.1", hj::REGEX_DECIMALS1));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_DECIMALS1));

    ASSERT_TRUE(std::regex_match("123.45", hj::REGEX_DECIMALS2));
    ASSERT_TRUE(std::regex_match("-123.45", hj::REGEX_DECIMALS2));
    ASSERT_TRUE(std::regex_match("123", hj::REGEX_DECIMALS2));
    ASSERT_TRUE(std::regex_match("0.00", hj::REGEX_DECIMALS2));
    ASSERT_FALSE(std::regex_match("123.456", hj::REGEX_DECIMALS2));
    ASSERT_FALSE(std::regex_match("123.ab", hj::REGEX_DECIMALS2));
    ASSERT_FALSE(std::regex_match("ab.12", hj::REGEX_DECIMALS2));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_DECIMALS2));

    ASSERT_TRUE(std::regex_match("123.456", hj::REGEX_DECIMALS3));
    ASSERT_TRUE(std::regex_match("-123.456", hj::REGEX_DECIMALS3));
    ASSERT_TRUE(std::regex_match("123", hj::REGEX_DECIMALS3));
    ASSERT_TRUE(std::regex_match("0.000", hj::REGEX_DECIMALS3));
    ASSERT_FALSE(std::regex_match("123.4567", hj::REGEX_DECIMALS3));
    ASSERT_FALSE(std::regex_match("123.abc", hj::REGEX_DECIMALS3));
    ASSERT_FALSE(std::regex_match("abc.123", hj::REGEX_DECIMALS3));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_DECIMALS3));

    ASSERT_TRUE(std::regex_match("a123", hj::REGEX_NUM_LATTER));
    ASSERT_TRUE(std::regex_match("A123b", hj::REGEX_NUM_LATTER));
    ASSERT_TRUE(std::regex_match("1234567890", hj::REGEX_NUM_LATTER));
    ASSERT_TRUE(std::regex_match("abcXYZ", hj::REGEX_NUM_LATTER));
    ASSERT_FALSE(std::regex_match("123.", hj::REGEX_NUM_LATTER));
    ASSERT_FALSE(std::regex_match(".A123b", hj::REGEX_NUM_LATTER));
    ASSERT_FALSE(std::regex_match("123.b", hj::REGEX_NUM_LATTER));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_NUM_LATTER));

    ASSERT_TRUE(std::regex_match("192.168.0.1", hj::REGEX_IP_V4));
    ASSERT_TRUE(std::regex_match("255.255.255.255", hj::REGEX_IP_V4));
    ASSERT_TRUE(std::regex_match("0.0.0.0", hj::REGEX_IP_V4));
    ASSERT_FALSE(std::regex_match("256.255.255.255", hj::REGEX_IP_V4));
    ASSERT_FALSE(std::regex_match("-1.0.0.0", hj::REGEX_IP_V4));
    ASSERT_FALSE(std::regex_match("0.0.0.0.0", hj::REGEX_IP_V4));
    ASSERT_FALSE(std::regex_match("0.0.0", hj::REGEX_IP_V4));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_IP_V4));

    ASSERT_TRUE(std::regex_match("0", hj::REGEX_PORT));
    ASSERT_TRUE(std::regex_match("65535", hj::REGEX_PORT));
    ASSERT_TRUE(std::regex_match("12345", hj::REGEX_PORT));
    ASSERT_FALSE(std::regex_match("-1", hj::REGEX_PORT));
    ASSERT_FALSE(std::regex_match("65536", hj::REGEX_PORT));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_PORT));

    ASSERT_TRUE(std::regex_match("DC-21-5C-03-04-1D", hj::REGEX_MAC));
    ASSERT_TRUE(std::regex_match("00-00-00-00-00-00", hj::REGEX_MAC));
    ASSERT_TRUE(std::regex_match("FF-FF-FF-FF-FF-FF", hj::REGEX_MAC));
    ASSERT_FALSE(std::regex_match("DC-21-5C-03-04", hj::REGEX_MAC));
    ASSERT_FALSE(std::regex_match("21-5C-03-04-1D", hj::REGEX_MAC));
    ASSERT_FALSE(std::regex_match("DC-21-5C-03-04-1D.", hj::REGEX_MAC));
    ASSERT_FALSE(std::regex_match("", hj::REGEX_MAC));
}