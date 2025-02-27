#include <gtest/gtest.h>
#include <libcpp/algo/regex.hpp>

TEST(regex, match)
{
    ASSERT_EQ(std::regex_match("432503199408097040", std::regex(libcpp::regex::IDCARD_CN)), true);
    ASSERT_EQ(std::regex_match("432503199408097040abc", std::regex(libcpp::regex::IDCARD_CN)), false);
    ASSERT_EQ(std::regex_match("43250319940809704", std::regex(libcpp::regex::IDCARD_CN)), false);
    ASSERT_EQ(std::regex_match("32503199408097040", std::regex(libcpp::regex::IDCARD_CN)), false);

    ASSERT_EQ(std::regex_match("123", std::regex(libcpp::regex::INTEGER)), true);
    ASSERT_EQ(std::regex_match("123.", std::regex(libcpp::regex::INTEGER)), false);
    ASSERT_EQ(std::regex_match("a123", std::regex(libcpp::regex::INTEGER)), false);
    ASSERT_EQ(std::regex_match("123b", std::regex(libcpp::regex::INTEGER)), false);
    ASSERT_EQ(std::regex_match("123.1", std::regex(libcpp::regex::INTEGER)), false);

    ASSERT_EQ(std::regex_match("123.4", std::regex(libcpp::regex::DECIMALS1)), true);
    ASSERT_EQ(std::regex_match("123.45", std::regex(libcpp::regex::DECIMALS1)), false);ASSERT_EQ(std::regex_match("123.a", std::regex(libcpp::regex::DECIMALS1)), false);
    ASSERT_EQ(std::regex_match("a.1", std::regex(libcpp::regex::DECIMALS1)), false);

    ASSERT_EQ(std::regex_match("123.45", std::regex(libcpp::regex::DECIMALS2)), true);
    ASSERT_EQ(std::regex_match("123.456", std::regex(libcpp::regex::DECIMALS2)), false);ASSERT_EQ(std::regex_match("123.ab", std::regex(libcpp::regex::DECIMALS2)), false);
    ASSERT_EQ(std::regex_match("ab.12", std::regex(libcpp::regex::DECIMALS2)), false);

    ASSERT_EQ(std::regex_match("123.456", std::regex(libcpp::regex::DECIMALS3)), true);
    ASSERT_EQ(std::regex_match("123.4567", std::regex(libcpp::regex::DECIMALS3)), false);ASSERT_EQ(std::regex_match("123.abc", std::regex(libcpp::regex::DECIMALS3)), false);
    ASSERT_EQ(std::regex_match("abc.123", std::regex(libcpp::regex::DECIMALS3)), false);

    ASSERT_EQ(std::regex_match("a123", std::regex(libcpp::regex::NUM_LATTER)), true);
    ASSERT_EQ(std::regex_match("a123b", std::regex(libcpp::regex::NUM_LATTER)), true);
    ASSERT_EQ(std::regex_match("A123", std::regex(libcpp::regex::NUM_LATTER)), true);
    ASSERT_EQ(std::regex_match("A123b", std::regex(libcpp::regex::NUM_LATTER)), true);
    ASSERT_EQ(std::regex_match("123.", std::regex(libcpp::regex::NUM_LATTER)), false);
    ASSERT_EQ(std::regex_match(".A123b", std::regex(libcpp::regex::NUM_LATTER)), false);
    ASSERT_EQ(std::regex_match("123.b", std::regex(libcpp::regex::NUM_LATTER)), false);

    ASSERT_EQ(std::regex_match("192.168.0.1", std::regex(libcpp::regex::IP_V4)), true);
    ASSERT_EQ(std::regex_match("255.255.255.255", std::regex(libcpp::regex::IP_V4)), true);
    ASSERT_EQ(std::regex_match("0.0.0.0", std::regex(libcpp::regex::IP_V4)), true);
    ASSERT_EQ(std::regex_match("256.255.255.255", std::regex(libcpp::regex::IP_V4)), false);
    ASSERT_EQ(std::regex_match("-1.0.0.0", std::regex(libcpp::regex::IP_V4)), false);
    ASSERT_EQ(std::regex_match("0.0.0.0.0", std::regex(libcpp::regex::IP_V4)), false);
    ASSERT_EQ(std::regex_match("0.0.0", std::regex(libcpp::regex::IP_V4)), false);

    ASSERT_EQ(std::regex_match("0", std::regex(libcpp::regex::PORT)), true);
    ASSERT_EQ(std::regex_match("65535", std::regex(libcpp::regex::PORT)), true);
    ASSERT_EQ(std::regex_match("-1", std::regex(libcpp::regex::PORT)), false);
    ASSERT_EQ(std::regex_match("65536", std::regex(libcpp::regex::PORT)), false);

    ASSERT_EQ(std::regex_match("DC-21-5C-03-04-1D", std::regex(libcpp::regex::MAC)), true);
    ASSERT_EQ(std::regex_match("DC-21-5C-03-04", std::regex(libcpp::regex::MAC)), false);
    ASSERT_EQ(std::regex_match("21-5C-03-04-1D", std::regex(libcpp::regex::MAC)), false);
    ASSERT_EQ(std::regex_match("DC-21-5C-03-04-1D.", std::regex(libcpp::regex::MAC)), false);
}